#include "coordinates.hh"

GeocentricCoordinates::GeocentricCoordinates(int year,
                                             int month,
                                             double day,
                                             std::string field_datafile,
                                             double vxgse,
                                             double vygse,
                                             double vzgse)
{
    // time information
    this->year = year;
    this->month = month;
    this->day = day;
    this->ut = datetime_to_utc(year, month, day);
    this->year_frac = datetime_to_yearfrac(year, month, day);

    this->vxgse = vxgse;
    this->vygse = vygse;
    this->vzgse = vzgse;

    this->field_datafile = field_datafile;
    LoadFieldData();

    Sun();
    InitTransformationMatrix();
}

GeocentricCoordinates::~GeocentricCoordinates()
{
}

void GeocentricCoordinates::LoadFieldData()
{
    std::vector<std::vector<std::vector<G4double>>> g;
    std::vector<std::vector<std::vector<G4double>>> h;
    std::vector<G4double> times;

    // Load field data from file
    // Open the data file
    std::ifstream file(field_datafile);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << field_datafile << G4endl;
        return;
    }

    std::string line;

    while (std::getline(file, line))
    { // Skip the lines that start with #
        if (line[0] != '#')
            break;
    }

    // 1. Load metadata from first line
    std::stringstream ss(line);
    ss >> N_MIN >> N_MAX >> NTIMES >> SP_ORDER >> N_STEPS;
    // initialize the g and h vectors to be of size (N_MAX+1) x (N_MAX+1) x NTIMES
    g.resize(N_MAX + 1, std::vector<std::vector<G4double>>(N_MAX + 1, std::vector<G4double>(NTIMES, 0)));
    h.resize(N_MAX + 1, std::vector<std::vector<G4double>>(N_MAX + 1, std::vector<G4double>(NTIMES, 0)));

    // 2. Loading the time points
    if (std::getline(file, line))
    {
        // turn the line into a stream
        std::stringstream ss(line);
        for (double v; ss >> v;)
        {
            times.push_back(v);
        }
    }

    // 3. Loading the Gauss coefficients for the Spherical Harmonic
    while (std::getline(file, line))
    {
        // turn the line into a stream
        std::stringstream ss(line);
        G4int n, m;
        ss >> n >> m;

        if (m >= 0)
        {
            for (G4int t = 0; t < NTIMES; t++)
            {
                ss >> g[n][m][t];
            }
        }
        else
        {
            m = -m;
            for (G4int t = 0; t < NTIMES; t++)
            {
                ss >> h[n][m][t];
            }
        }
    }

    file.close();

    // 4. Check if the input year is within the valid range
    if (year_frac < times.front() || year_frac > times.back())
    {
        G4cerr << "Error: year input " << year_frac << " is out of valid input range (" << times.front() << ", " << times.back() << ")" << G4endl;
        return;
    }

    // 5. Find t1 (the time point just before the input year) and
    //    t2 (the time point just after the input year)
    int t_1_index, t_2_index;
    G4double t_1, t_2;

    for (int i = 1; i < NTIMES; i++)
    {
        if (times[i] > year_frac)
        {
            t_2_index = i;
            t_1_index = i - 1;
            break;
        }
    }
    t_1 = times[t_1_index];
    t_2 = times[t_2_index];

    // 6. Interpolate the Gauss coefficients for the input year
    g_interp.resize(N_MAX + 1, std::vector<G4double>(N_MAX + 1, 0));
    h_interp.resize(N_MAX + 1, std::vector<G4double>(N_MAX + 1, 0));
    for (int n = 0; n <= N_MAX; n++)
    {
        for (int m = 0; m <= N_MAX; m++)
        {
            g_interp[n][m] = g[n][m][t_1_index] + (g[n][m][t_2_index] - g[n][m][t_1_index]) / (t_2 - t_1) * (year_frac - t_1);
            h_interp[n][m] = h[n][m][t_1_index] + (h[n][m][t_2_index] - h[n][m][t_1_index]) / (t_2 - t_1) * (year_frac - t_1);
        }
    }
}

void GeocentricCoordinates::Sun()
{
    /*
    Calculates four quantities necessary for coordinate transformations
    which depend on sun position (and, hence, on universal time and season):
        gst - greenwich mean sidereal time, slong - longitude along ecliptic
        srasn - right ascension,  sdec - declination of the sun (radians)
        obliq - mean oblique of the ecliptic (radian)

    Based on http://aa.usno.navy.mil/faq/docs/SunApprox.php and http://aa.usno.navy.mil/faq/docs/GAST.php
    Based on geopack by Sheng Tian: https://github.com/tsssss/geopack
    */
    double twopi = 2 * PI;
    double jd2000 = 2451545.0;
    // convert to Julian date.
    double t0_jd = 2440587.5;             // in day, 0 of Julian day.
    double secofdaygsw_x = 1.0 / 86400.0; // 1/sec of day.
    double t_jd = ut * secofdaygsw_x + t0_jd;

    // d = mjd - mj2000.
    double d = t_jd - jd2000;

    // mean obliquity of the ecliptic
    epsilon = 0.4090877233749509 - 6.2831853e-9 * d;

    // mean anomaly of the Sun, g.
    double g = 6.2400582213628066 + 0.0172019699945780 * d;
    g = fmod(g, twopi);

    // mean longitude of the Sun, q.
    double q = 4.8949329668507771 + 0.0172027916955899 * d;
    q = fmod(q, twopi);

    // geocentric apparent ecliptic longitude, l.
    slong = q + 0.0334230551756914 * sin(g) + 0.0003490658503989 * sin(2 * g);

    double sind = sin(epsilon) * sin(slong);
    sdec = asin(sind);
    srasn = fmod(atan2(cos(epsilon) * sin(slong), cos(slong)), twopi);

    // http://aa.usno.navy.mil/faq/docs/GAST.php
    // gst - gmst, greenwich mean sidereal time.
    gmst = fmod((4.894961212735792 + 6.30038809898489 * d), twopi);
    sgst = sin(gmst);
    cgst = cos(gmst);
}

void GeocentricCoordinates::InitTransformationMatrix()
{
    /*
    Initializes the transformation matrix from GSE to GEO coordinates,
    and from GEO to GSE coordinates.
    Based on geopack by Sheng Tian: https://github.com/tsssss/geopack
    */
    // coefficients for a given time, g_n^m(t), h_n^m(t)
    double g10 = -g_interp[1][0];
    double g11 = -g_interp[1][1];
    double h11 = -h_interp[1][1];

    //  Now calculate the components of the unit vector ezmag in geo coord.system:
    //  sin(teta0)*cos(lambda0), sin(teta0)*sin(lambda0), and cos(teta0)
    //        st0 * cl0                st0 * sl0                ct0 
    double sq=g11*g11+h11*h11;
    double sqq=sqrt(sq);
    double sqr=sqrt(g10*g10+sq);
    double sl0= h11/sqq;
    double cl0= g11/sqq;
    double st0= sqq/sqr;
    double ct0= g10/sqr;
    double stcl=st0*cl0;
    double stsl=st0*sl0;
    double ctsl=ct0*sl0;
    double ctcl=ct0*cl0;

    // All vectors are expressed in GEI.
        
    // xgse_[xyz] (s[123]) are the components of the unit vector exgsm=exgse in GEI,
    // pointing from the earth's center to the sun:
    double xgse_x= cos(srasn)*cos(sdec);
    double xgse_y= sin(srasn)*cos(sdec);
    double xgse_z= sin(sdec);

    // zgse_[xyz] (dz[123]) in GEI has the components (0,-sin(delta),cos(delta)) = (0.,-0.397823,0.917462);
    // Here delta = 23.44214 deg for the epoch 1978 (see the book by gurevich or other astronomical handbooks).
    // Here the most accurate time-dependent formula is used:
    double zgse_x=0.;
    double zgse_y=-sin(epsilon);
    double zgse_z= cos(epsilon);

    // ygse_[xyz] (dy[123]) = zgse_[xyz] x xgsm_[xyz] in GEI:
    double ygse_x=zgse_y*xgse_z-zgse_z*xgse_y;
    double ygse_y=zgse_z*xgse_x-zgse_x*xgse_z;
    double ygse_z=zgse_x*xgse_y-zgse_y*xgse_x;

    // zsm_[xyz] (dip[123]) are the components of the unit vector zsm=zmag in GEI:
    double zsm_x=stcl*cgst-stsl*sgst;
    double zsm_y=stcl*sgst+stsl*cgst;
    double zsm_z=ct0;

    // xgsw_[xyz] (x[123]) in GEI.
    double v1 = -1.0/sqrt(vxgse*vxgse+vygse*vygse+vzgse*vzgse);
    double xgsw_x = (vxgse*xgse_x + vygse*ygse_x + vzgse*zgse_x)*v1;
    double xgsw_y = (vxgse*xgse_y + vygse*ygse_y + vzgse*zgse_y)*v1;
    double xgsw_z = (vxgse*xgse_z + vygse*ygse_z + vzgse*zgse_z)*v1;

    // ygsw (y[123]) = zsm x xgsw in GEI.
    double ygsw_x=zsm_y*xgsw_z-zsm_z*xgsw_y;
    double ygsw_y=zsm_z*xgsw_x-zsm_x*xgsw_z;
    double ygsw_z=zsm_x*xgsw_y-zsm_y*xgsw_x;
    double y=sqrt(ygsw_x*ygsw_x+ygsw_y*ygsw_y+ygsw_z*ygsw_z);
    ygsw_x=ygsw_x/y;
    ygsw_y=ygsw_y/y;
    ygsw_z=ygsw_z/y;

    // zgsw (z[123]) = xgsw x ygsw in GEI.
    double zgsw_x = xgsw_y*ygsw_z-xgsw_z*ygsw_y;
    double zgsw_y = xgsw_z*ygsw_x-xgsw_x*ygsw_z;
    double zgsw_z = xgsw_x*ygsw_y-xgsw_y*ygsw_x;

    // xgsm = xgse in GEI.
    double xgsm_x = xgse_x;
    double xgsm_y = xgse_y;
    double xgsm_z = xgse_z;

    // ygsm = zsm x xgsm in GEI.
    double ygsm_x = zsm_y*xgsm_z - zsm_z*xgsm_y;
    double ygsm_y = zsm_z*xgsm_x - zsm_x*xgsm_z;
    double ygsm_z = zsm_x*xgsm_y - zsm_y*xgsm_x;
    y=sqrt(ygsm_x*ygsm_x+ygsm_y*ygsm_y+ygsm_z*ygsm_z);
    ygsm_x = ygsm_x/y;
    ygsm_y = ygsm_y/y;
    ygsm_z = ygsm_z/y;

    // ezgsm = exgsm x eygsm in GEI.
    double zgsm_x = xgse_y*ygsm_z-xgse_z*ygsm_y;
    double zgsm_y = xgse_z*ygsm_x-xgse_x*ygsm_z;
    double zgsm_z = xgse_x*ygsm_y-xgse_y*ygsm_x;
    
    // elements of the matrix gsm to gsw are the scalar products:
    // e11 = (exgsm, exgsw) e12 = (exgsm, eygsw) e13 = (exgsm, ezgsw)
    // e21 = (eygsm, exgsw) e22 = (eygsm, eygsw) e23 = (eygsm, ezgsw)
    // e31 = (ezgsm, exgsw) e32 = (ezgsm, eygsw) e33 = (ezgsm, ezgsw)
    e11 = xgsm_x*xgsw_x + xgsm_y*xgsw_y + xgsm_z*xgsw_z;
    e12 = xgsm_x*ygsw_x + xgsm_y*ygsw_y + xgsm_z*ygsw_z;
    e13 = xgsm_x*zgsw_x + xgsm_y*zgsw_y + xgsm_z*zgsw_z;
    e21 = ygsm_x*xgsw_x + ygsm_y*xgsw_y + ygsm_z*xgsw_z;
    e22 = ygsm_x*ygsw_x + ygsm_y*ygsw_y + ygsm_z*ygsw_z;
    e23 = ygsm_x*zgsw_x + ygsm_y*zgsw_y + ygsm_z*zgsw_z;
    e31 = zgsm_x*xgsw_x + zgsm_y*xgsw_y + zgsm_z*xgsw_z;
    e32 = zgsm_x*ygsw_x + zgsm_y*ygsw_y + zgsm_z*ygsw_z;
    e33 = zgsm_x*zgsw_x + zgsm_y*zgsw_y + zgsm_z*zgsw_z;
    
    // Tilt angle: psi=arcsin(ezsm dot exgsm)
    double sps=zsm_x*xgse_x+zsm_y*xgse_y+zsm_z*xgse_z;
    double cps=sqrt(1.-sps*sps);
    double psi=asin(sps);

    // The elements of the matrix mag to sm are the scalar products:
    // cfi=gm22=(eysm,eymag), sfi=gm23=(eysm,exmag); They can be derived as follows:
    // In geo the vectors exmag and eymag have the components (ct0*cl0,ct0*sl0,-st0) and (-sl0,cl0,0), respectively.

    // Hence, in gei the components are:
    //     exmag:    ct0*cl0*cos(gst)-ct0*sl0*sin(gst)
    //               ct0*cl0*sin(gst)+ct0*sl0*cos(gst)
    //               -st0
    //     eymag:    -sl0*cos(gst)-cl0*sin(gst)
    //               -sl0*sin(gst)+cl0*cos(gst)
    //                0
    // The components of eysm in gei were found above as ysm_in_geix, ysm_in_geiy, and ysm_in_geiz;
    // Now we only have to combine the quantities into scalar products:
    double xmag_x= ct0*(cl0*cgst-sl0*sgst);
    double xmag_y= ct0*(cl0*sgst+sl0*cgst);
    double xmag_z=-st0;
    double ymag_x=-(sl0*cgst+cl0*sgst);
    double ymag_y=-(sl0*sgst-cl0*cgst);
    double cfi=ygsm_x*ymag_x+ygsm_y*ymag_y;
    double sfi=ygsm_x*xmag_x+ygsm_y*xmag_y+ygsm_z*xmag_z;

    double xmut=(atan2(sfi,cfi)+3.1415926536)*3.8197186342;

    // The elements of the matrix geo to gsm are the scalar products:
    // a11=(exgeo,exgsm), a12=(eygeo,exgsm), a13=(ezgeo,exgsm),
    // a21=(exgeo,eygsm), a22=(eygeo,eygsm), a23=(ezgeo,eygsm),
    // a31=(exgeo,ezgsm), a32=(eygeo,ezgsm), a33=(ezgeo,ezgsm),
    // All the unit vectors in brackets are already defined in gei:
    // xgeo=(cgst,sgst,0), ygeo=(-sgst,cgst,0), zgeo=(0,0,1)
    // and therefore:
    a11= xgsm_x*cgst+xgse_y*sgst;
    a12=-xgsm_x*sgst+xgse_y*cgst;
    a13= xgsm_z;
    a21= ygsm_x *cgst+ygsm_y *sgst;
    a22=-ygsm_x *sgst+ygsm_y *cgst;
    a23= ygsm_z;
    a31= zgsm_x*cgst+zgsm_y*sgst;
    a32=-zgsm_x*sgst+zgsm_y*cgst;
    a33= zgsm_z;

    // The elements of the matrix geo to gsw:
    b11 = e11*a11+e21*a21+e31*a31;
    b12 = e11*a12+e21*a22+e31*a32;
    b13 = e11*a13+e21*a23+e31*a33;
    b21 = e12*a11+e22*a21+e32*a31;
    b22 = e12*a12+e22*a22+e32*a32;
    b23 = e12*a13+e22*a23+e32*a33;
    b31 = e13*a11+e23*a21+e33*a31;
    b32 = e13*a12+e23*a22+e33*a32;
    b33 = e13*a13+e23*a23+e33*a33;  
}

void GeocentricCoordinates::gsm_to_gsw(const G4double* point_gsm, G4double* point_gsw) const
{
    G4double x_gsm = point_gsm[0];
    G4double y_gsm = point_gsm[1];
    G4double z_gsm = point_gsm[2];
    point_gsw[0] = x_gsm * e11 + y_gsm * e21 + z_gsm * e31;
    point_gsw[1] = x_gsm * e12 + y_gsm * e22 + z_gsm * e32;
    point_gsw[2] = x_gsm * e13 + y_gsm * e23 + z_gsm * e33;
}

void GeocentricCoordinates::gsw_to_gsm(const G4double* point_gsw, G4double* point_gsm) const
{
    G4double x_gsw = point_gsw[0];
    G4double y_gsw = point_gsw[1];
    G4double z_gsw = point_gsw[2];
    point_gsm[0] = x_gsw * e11 + y_gsw * e12 + z_gsw * e13;
    point_gsm[1] = x_gsw * e21 + y_gsw * e22 + z_gsw * e23;
    point_gsm[2] = x_gsw * e31 + y_gsw * e32 + z_gsw * e33;
}

void GeocentricCoordinates::geo_to_gsm(const G4double* point_geo, G4double* point_gsm) const
{
    G4double x_geo = point_geo[0];
    G4double y_geo = point_geo[1];
    G4double z_geo = point_geo[2];
    point_gsm[0] = x_geo * a11 + y_geo * a12 + z_geo * a13;
    point_gsm[1] = x_geo * a21 + y_geo * a22 + z_geo * a23;
    point_gsm[2] = x_geo * a31 + y_geo * a32 + z_geo * a33;
}

void GeocentricCoordinates::gsm_to_geo(const G4double* point_gsm, G4double* point_geo) const
{
    G4double x_gsm = point_gsm[0];
    G4double y_gsm = point_gsm[1];
    G4double z_gsm = point_gsm[2];
    point_geo[0] = x_gsm * a11 + y_gsm * a21 + z_gsm * a31;
    point_geo[1] = x_gsm * a12 + y_gsm * a22 + z_gsm * a32;
    point_geo[2] = x_gsm * a13 + y_gsm * a23 + z_gsm * a33;
}

void GeocentricCoordinates::geo_to_gsw(const G4double* point_geo, G4double* point_gsw) const
{
    G4double x_geo = point_geo[0];
    G4double y_geo = point_geo[1];
    G4double z_geo = point_geo[2];
    point_gsw[0] = x_geo * b11 + y_geo * b12 + z_geo * b13;
    point_gsw[1] = x_geo * b21 + y_geo * b22 + z_geo * b23;
    point_gsw[2] = x_geo * b31 + y_geo * b32 + z_geo * b33;
}

void GeocentricCoordinates::gsw_to_geo(const G4double* point_gsw, G4double* point_geo) const
{
    G4double x_gsw = point_gsw[0];
    G4double y_gsw = point_gsw[1];
    G4double z_gsw = point_gsw[2];
    point_geo[0] = x_gsw * b11 + y_gsw * b21 + z_gsw * b31;
    point_geo[1] = x_gsw * b12 + y_gsw * b22 + z_gsw * b32;
    point_geo[2] = x_gsw * b13 + y_gsw * b23 + z_gsw * b33;
}