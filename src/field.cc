#include "field.hh"

TabulatedEarthMagneticField::TabulatedEarthMagneticField(const G4String datafile)
{
    // Constructor
    // Assume the data file is a CSV with columns: r, theta, phi, Bx, By, Bz
    // and that the grid is regular in r, phi, theta
    std::ifstream file(datafile);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << datafile << G4endl;
        return;
    }

    std::string line;

    // Read the first line to get the header information
    std::getline(file, line);
    // TODO: check header format

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string numstr;

        // Read the spherical coordinates
        std::getline(ss, numstr, ',');
        G4double r = std::stod(numstr);
        std::getline(ss, numstr, ',');
        G4double theta = std::stod(numstr);
        std::getline(ss, numstr, ',');
        G4double phi = std::stod(numstr);

        // Read the magnetic field components in nT
        std::getline(ss, numstr, ',');
        G4double Bx = std::stod(numstr) * 1e-5 * gauss; // Convert from nT to Gauss
        std::getline(ss, numstr, ',');
        G4double By = std::stod(numstr) * 1e-5 * gauss;
        std::getline(ss, numstr, ',');
        G4double Bz = std::stod(numstr) * 1e-5 * gauss;

        BFieldTable.push_back({Bx, By, Bz});
    }
    file.close();

    this->r_min = 6371.2 * km;
    this->r_max = 6371.2 * km + 1000 * km;
    this->theta_min = 0;
    this->theta_max = 3.14159265358979323846;
    this->phi_min = 0;
    this->phi_max = 2 * 3.14159265358979323846;
    this->n_r = 20;
    this->n_theta = 10;
    this->n_phi = 20;

    // Compute grid intervals
    this->dr = (r_max - r_min) / (n_r - 1);
    this->dtheta = (theta_max - theta_min) / (n_theta - 1);
    this->dphi = (phi_max - phi_min) / (n_phi - 1);
}

TabulatedEarthMagneticField::~TabulatedEarthMagneticField()
{
    // Destructors
}

void TabulatedEarthMagneticField::GetFieldValue(const G4double *Point, G4double *Bfield) const
{
    G4double x = Point[0];
    G4double y = Point[1];
    G4double z = Point[2];

    // Convert position to spherical coordinates
    G4double r = sqrt(x * x + y * y + z * z);
    G4double theta = acos(z / r);
    G4double phi = atan2(y, x);

    // check if point is within the defined region
    if (r <= r_min || r >= r_max ||
        theta <= theta_min || theta >= theta_max ||
        phi <= phi_min || phi >= phi_max)
    {
        Bfield[0] = 0;
        Bfield[1] = 0;
        Bfield[2] = 0;
        return;
    }

    // Find the indices of the grid cell containing the point
    // (finding the "lower" corner of the cell is sufficient)
    G4int ind_r = static_cast<G4int>(std::floor((r - r_min) / dr));
    G4int ind_theta = static_cast<G4int>(std::floor((theta - theta_min) / dtheta));
    G4int ind_phi = static_cast<G4int>(std::floor((phi - phi_min) / dphi));

    // Position of the point in the grid, normalized to [0, 1]
    G4double r_frac = (r - (r_min + ind_r * dr)) / dr;
    G4double theta_frac = (theta - (theta_min + ind_theta * dtheta)) / dtheta;
    G4double phi_frac = (phi - (phi_min + ind_phi * dphi)) / dphi;

    // Get the values for the 8 corners of the cell
    // reference: https://en.wikipedia.org/wiki/Trilinear_interpolation
    // Trilinear interpolation
    for (int xyz = 0; xyz < 3; xyz++)
    {
        G4double c000 = BFieldTable[(ind_r * n_theta * n_phi) + (ind_theta * n_phi) + ind_phi][xyz];
        G4double c100 = BFieldTable[((ind_r + 1) * n_theta * n_phi) + (ind_theta * n_phi) + ind_phi][xyz];
        G4double c010 = BFieldTable[(ind_r * n_theta * n_phi) + ((ind_theta + 1) * n_phi) + ind_phi][xyz];
        G4double c110 = BFieldTable[((ind_r + 1) * n_theta * n_phi) + ((ind_theta + 1) * n_phi) + ind_phi][xyz];
        G4double c001 = BFieldTable[(ind_r * n_theta * n_phi) + (ind_theta * n_phi) + (ind_phi + 1)][xyz];
        G4double c101 = BFieldTable[((ind_r + 1) * n_theta * n_phi) + (ind_theta * n_phi) + (ind_phi + 1)][xyz];
        G4double c011 = BFieldTable[(ind_r * n_theta * n_phi) + ((ind_theta + 1) * n_phi) + (ind_phi + 1)][xyz];
        G4double c111 = BFieldTable[((ind_r + 1) * n_theta * n_phi) + ((ind_theta + 1) * n_phi) + (ind_phi + 1)][xyz];

        Bfield[xyz] = trilinear_interp(r_frac, theta_frac, phi_frac,
                                       c000, c100,
                                       c010, c110,
                                       c001, c101,
                                       c011, c111);
    }
}

IGRF::IGRF(const std::string datafile,
                                       G4int year,
                                       G4int month,
                                       G4int day,
                                       int N_MAX)
{
    // Constructor
    year_frac = datetime_to_yearfrac(year, month, day);

    LoadData(datafile);

    // overwrite N_MAX if specified
    if (N_MAX != -1 && N_MAX <= this->N_MAX)
    {
        this->N_MAX = N_MAX;
    }
}

IGRF::~IGRF()
{
    // Destructor
}

void IGRF::LoadData(const std::string datafile)
{
    // Open the data file
    std::ifstream file(datafile);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << datafile << G4endl;
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

void IGRF::GetFieldValue(const G4double *Point, G4double *Bfield) const
{
    G4double x = Point[0];
    G4double y = Point[1];
    G4double z = Point[2];

    // Convert position to spherical coordinates
    G4double r = sqrt(x * x + y * y + z * z);
    G4double theta = acos(z / r);
    G4double phi = atan2(y, x);

    std::vector<std::vector<double>> P(N_MAX + 1, std::vector<double>(N_MAX + 1, 0));
    std::vector<std::vector<double>> dP(N_MAX + 1, std::vector<double>(N_MAX + 1, 0));

    associated_legendre(N_MAX, N_MAX, theta, P, dP);

    G4double Br = 0.0;
    G4double Btheta = 0.0;
    G4double Bphi = 0.0;

    for (int n = 1; n <= N_MAX; n++)
    {
        G4double r_term = pow(RE / r, n + 2);
        for (int m = 0; m <= n; m++)
        {
            G4double phi_term = (g_interp[n][m] * cos(m * phi) + h_interp[n][m] * sin(m * phi));
            Br += (n + 1) * r_term * phi_term * P[n][m];
            Btheta += -r_term * phi_term * dP[n][m];
            Bphi += -r_term * (-(m * g_interp[n][m] * sin(m * phi)) + (m * h_interp[n][m] * cos(m * phi))) * P[n][m];
        }
    }
    Bphi /= sin(theta);

    // Convert from nT to Gauss
    Br = Br * 1e-5 * gauss;
    Btheta = Btheta * 1e-5 * gauss;
    Bphi = Bphi * 1e-5 * gauss;
    G4cout << Br / gauss / 1e-5 << " " << Btheta / gauss / 1e-5 << " " << Bphi / gauss / 1e-5 << G4endl;

    // Convert from spherical to Cartesian coordinates
    Bfield[0] = sin(theta) * cos(phi) * Br + cos(theta) * cos(phi) * Btheta - sin(phi) * Bphi; // Bx
    Bfield[1] = sin(theta) * sin(phi) * Br + cos(theta) * sin(phi) * Btheta + cos(phi) * Bphi; // By
    Bfield[2] = cos(theta) * Br - sin(theta) * Btheta;                                         // Bz
}

TA16::TA16(int year, int month, int day, G4double pdyn, G4double symhc, G4double xind, G4double by_imf, G4double ps, double vxgse, double vygse, double vzgse, std::string param_file, std::string igrf_datafile, int N_MAX)
{
    this->pdyn = pdyn;
    this->symhc = symhc;
    this->xind = xind;
    this->by_imf = by_imf;
    this->ps = ps / radian;

    this->year = year;
    this->month = month;
    this->day = day;

    // Read the const stored in 'TA16_RBF.par' into array a
    // MODEL LINEAR COEFFICIENTS A_i,B_i,.. IN THE POLO- AND TORO-EXPANSIONS
    std::ifstream infile(param_file);
    double buffer;
    while (infile >> buffer)
    {
        a.push_back(buffer);
    }
    infile.close();

    // IN THIS CODE WE ASSUME A SIMPLIFIED VERSION OF THE LIN ET AL MODEL (WITHOUT INDENTATIONS
    // AND MINOR TILT-INDEPENDENT ASYMMETRIES
    a_const[10] = 0.0;
    a_const[11] = 0.0;
    a_const[13] = 0.0;
    a_const[14] = 0.0;

    // Construct the RBF grid
    ConstructRBFGrid();
    // Precompute constants used in GetField_GSM
    PrecomputeGetFieldConstants();
    
    // Initialize inner field
    this->inner_field = new IGRF(igrf_datafile, year, month, day, N_MAX);

    // Initialize coordinate transformation 
    this->coord = new GeocentricCoordinates(year, month, day, igrf_datafile, vxgse, vygse, vzgse);
}

TA16::~TA16()
{
    // Destructor
}

void TA16::ConstructRBFGrid()
{
    // CREATE A 3D KURIHARA'S GRID OF RBF CENTERS (ONE-TIME-ONLY PROCEDURE)
    int klat = 8;                 // KLAT IS THE NUMBER OF LATITUDE CIRCLES IN THE NORTHERN HEMISPHERE (EXCLUDING THE POLE)
    double rlow_grid = 3.3;       // THE INNERMOST SPHERE RADIUS
    double rhigh_grid = 16.0;     //  UPPER LIMIT ON THE RADIUS OF OUTERMOST RBF SPHERE
    double xgsw_lim_data = -20.0; //  UPPER LIMIT ON THE DATA TAILWARD LIMIT

    double rh = 8.0;     //  HINGING PARAMETERS (SEE TAG-2015)
    double alpha = 3.0;  //  ALPHA PARAMETER (SEE TAG-2015)
    int nlat = klat + 1; // NLAT IS THE NUMBER OF LATITUDE CIRCLES IN THE NORTHERN HEMISPHERE
                         //      (INCLUDING THE POLE)

    double dlat_deg = 90.0 / (nlat - 0.5); // ANGULAR INTERVAL BETWEEN THE LATITUDE CIRCLES

    int l = 0;
    double r = rlow_grid; //  THE INNERMOST SPHERE RADIUS

    double pd_tr = 0.5; //  this is just to filter out RBF centers outside the Lin et al model magnetopause
    double pm = 0.0;
    double BzIMF_TR = 0.0;
    double psi = 0.0;

    for (int j = 0; j < 100; j++)
    { // J COUNTS THE NUMBER OF SPHERES WITH RBF CENTERS (WHICH IS ACTUALLY MUCH LESS THAN 100)
        for (int i = 0; i < nlat; i++)
        {                                  // I COUNTS THE LATITUDE CIRCLES (FROM NORTH POLE DOWN TO EQUATOR)
            double xcolatd = dlat_deg * i; //  COLATITUDE OF THE Ith CIRCLE (in degs)
            int nlon = 4 * i;              //  NUMBER OF RBF CENTERS ON THE ItH CIRCLE
            double dlon_deg;
            if (i != 0)
            {
                dlon_deg = 360.0 / nlon; //  LONGITUDE INTERVAL BETWEEN CENTERS (degs)
            }
            else
            {
                nlon = 1;
                dlon_deg = 0.0;
            }

            for (int k = 0; k < nlon; k++)
            {
                double xlond = k * dlon_deg;
                double xcolat = xcolatd * (PI / 180.0); // COLATITUDE and
                double xlon = xlond * (PI / 180.0);     // LONGITUDE OF THE Lth RBF CENTER

                double xxxx = r * sin(xcolat) * cos(xlon);
                double yyyy = r * sin(xcolat) * sin(xlon);
                double zzzz = r * cos(xcolat);

                // HERE WE CALCULATE THE DISTANCE FROM THE RBF NODE TO THE LIN ET AL. [2010] MODEL MAGNETOPAUSE.
                // THE RBF NODES LYING OUTSIDE THE BOUNDARY ARE NOT INCLUDED IN THE GRID
                double en = a_const[21];
                double es = a_const[21];
                double thetan = a_const[19] + a_const[20] * psi;
                double thetas = a_const[19] - a_const[20] * psi;
                double ctn = cos(thetan);
                double cts = cos(thetas);
                double stn = sin(thetan);
                double sts = sin(thetas);

                double rho2 = yyyy * yyyy + zzzz * zzzz;
                double r1 = sqrt(xxxx * xxxx + rho2);
                double rho1 = sqrt(rho2);

                double ctt = xxxx / r1;
                double stt = sqrt(yyyy * yyyy + zzzz * zzzz) / r1;
                double t = atan2(stt, ctt);

                double sp, cp;
                if (rho1 > 1.0e-5)
                {
                    sp = zzzz / rho1;
                    cp = yyyy / rho1;
                }
                else
                {
                    if (xxxx > 0.0)
                    {
                        sp = 0.0;
                        cp = 1.0;
                    }
                    else
                    {
                        double rm = 1000.0;
                        G4cout << "Error in TA16. " << G4endl;
                        return;
                    }
                }

                double psin = acos(ctt * ctn + stt * stn * sp);
                double psis = acos(ctt * cts - stt * sts * sp);

                double dn = a_const[16] + (a_const[17] + a_const[18] * psi) * psi;
                double ds = a_const[16] - (a_const[17] - a_const[18] * psi) * psi;

                double cn = a_const[14] * pow((pd_tr + pm), a_const[15]);
                double cs = cn;

                double b0 = a_const[6] + a_const[7] * (exp(a_const[8] * BzIMF_TR) - 1.0) / (exp(a_const[9] * BzIMF_TR) + 1.0);
                double b1 = a_const[10];
                double b2 = a_const[11] + a_const[12] * psi;
                double b3 = a_const[13];

                double f1 = pow((sqrt(0.5 * (1.0 + ctt)) + a_const[5] * 2 * stt * ctt * (1 - exp(-t))), (b0 + b1 * cp + b2 * sp + b3 * sp * sp));
                double r0 = a_const[0] * pow((pd_tr + pm), a_const[1]) * (1.0 + a_const[2] * (exp(a_const[3] * BzIMF_TR) - 1.0) / (exp(a_const[4] * BzIMF_TR) + 1.0));
                double rm = r0 * f1 + cn * exp(dn * pow(psin, en)) + cs * exp(ds * pow(psis, es)); // POSITION OF THE MODEL MAGNETOPAUSE

                if (r > rm)
                {
                    continue;
                }

                l = l + 1; //  COUNTER OF THE RBF CENTERS
                xx[l - 1] = xxxx;
                yy[l - 1] = yyyy;
                zz[l - 1] = zzzz;
                st[l - 1] = sin(xcolat);
                rho[l - 1] = r * st[l - 1];
                zsp[l - 1] = zz[l - 1] * sin(xlon);
                zcp[l - 1] = zz[l - 1] * cos(xlon);
                rhbr[l - 1] = rh / r * (1.0 - pow(1.0 + pow(r / rh, alpha), 1.0 / alpha));
            }
        }
        double rlast = r;                                          // INCREMENT R BY A FIXED FACTOR
        r = r * (nlat - 0.5 + PI / 4.0) / (nlat - 0.5 - PI / 4.0); // CENTERS CREATED ONLY INSIDE R=RHIGH

        if (r > rhigh_grid)
        {
            break;
        }
    }
}

void TA16::PrecomputeGetFieldConstants()
{
    fpd = sqrt(pdyn / 2.0) - 1.0;
    symh = symhc / 50.0;
    cps = cos(ps);
    sps = sin(ps);
    tps = sps / cps;

    double act[1296], ast[1296], at[1296], acp[1296], asp[1296], ap[1296];
    for (int i = 0; i < 1296; i++)
    {
        double dtheta = -asin(rhbr[i] * tps) * st[i];
        double sdt = sin(dtheta);
        double cdtm1 = cos(dtheta) - 1.0;

        dxp_term[i] = -(1.0 + cdtm1) * xx[i] - sdt * zcp[i];
        dyp_term[i] = -(1.0 + cdtm1) * yy[i] - sdt * zsp[i];
        dzp_term[i] = -(1.0 + cdtm1) * zz[i] + rho[i] * sdt;
        dxm_term[i] = -(1.0 + cdtm1) * xx[i] + sdt * zcp[i];
        dym_term[i] = -(1.0 + cdtm1) * yy[i] + sdt * zsp[i];
        dzm_term[i] = (1.0 + cdtm1) * zz[i] + rho[i] * sdt;

        act[i] = a[i] + a[i + 5184] * fpd + a[i + 10368] * symh + a[i + 15552] * xind;
        ast[i] = a[i + 1296] + a[i + 6480] * fpd + a[i + 11664] * symh + a[i + 16848] * xind;
        at[i] = a[i + 20736] * by_imf;
        acp[i] = a[i + 2592] + a[i + 7776] * fpd + a[i + 12960] * symh + a[i + 18144] * xind;
        asp[i] = a[i + 3888] + a[i + 9072] * fpd + a[i + 14256] * symh + a[i + 19440] * xind;
        ap[i] = a[i + 22032] * by_imf;

        term1[i] = cps * act[i];
        term2[i] = (sps * ast[i] + at[i]);
        term3[i] = cps * acp[i];
        term4[i] = (sps * asp[i] + ap[i]);
    }
}

void TA16::GetField_GSM(const G4double *Point, G4double *Bfield) const
{
    double x = Point[0] / 6371.2 / km;
    double y = Point[1] / 6371.2 / km;
    double z = Point[2] / 6371.2 / km;

    // convert from gsm to sm coordinates
    double xsm = x * cps - z * sps;
    double ysm = y;
    double zsm = z * cps + x * sps;

    double bxsm = 0.0;
    double bysm = 0.0;
    double bzsm = 0.0;

    for (int i = 0; i < 1296; i++)
    {
        double xp_term = xsm + dxp_term[i];
        double yp_term = ysm + dyp_term[i];
        double zp_term = zsm + dzp_term[i];
        double xm_term = xsm + dxm_term[i];
        double ym_term = ysm + dym_term[i];
        double zm_term = zsm + dzm_term[i];

        double cp = sqrt(xp_term * xp_term + yp_term * yp_term + zp_term * zp_term + d * d); // RBF Ch_i+
        double cm = sqrt(xm_term * xm_term + ym_term * ym_term + zm_term * zm_term + d * d); // RBF Ch_i-
        double dcpx = xp_term / cp;
        double dcmx = xm_term / cm;
        double dcpy = yp_term / cp;
        double dcmy = ym_term / cm;
        double dcpz = zp_term / cp;
        double dcmz = zm_term / cm;

        double dcpx2 = 1.0 / cp - dcpx * dcpx / cp;
        double dcmx2 = 1.0 / cm - dcmx * dcmx / cm;
        double dcpy2 = 1.0 / cp - dcpy * dcpy / cp;
        double dcmy2 = 1.0 / cm - dcmy * dcmy / cm;
        double dcpz2 = 1.0 / cp - dcpz * dcpz / cp;
        double dcmz2 = 1.0 / cm - dcmz * dcmz / cm;
        double dcpxy = -dcpx * dcpy / cp;
        double dcmxy = -dcmx * dcmy / cm;
        double dcpxz = -dcpx * dcpz / cp;
        double dcmxz = -dcmx * dcmz / cm;
        double dcpyz = -dcpy * dcpz / cp;
        double dcmyz = -dcmy * dcmz / cm;

        double txcp = zsm * dcpy - ysm * dcpz;
        double tycp = xsm * dcpz - zsm * dcpx;
        double tzcp = ysm * dcpx - xsm * dcpy;
        double txcm = zsm * dcmy - ysm * dcmz;
        double tycm = xsm * dcmz - zsm * dcmx;
        double tzcm = ysm * dcmx - xsm * dcmy;

        double pxcp = 2.0 * dcpx - xsm * (dcpy2 + dcpz2) + ysm * dcpxy + zsm * dcpxz;
        double pycp = 2.0 * dcpy - ysm * (dcpx2 + dcpz2) + zsm * dcpyz + xsm * dcpxy;
        double pzcp = 2.0 * dcpz - zsm * (dcpx2 + dcpy2) + xsm * dcpxz + ysm * dcpyz;
        double pxcm = 2.0 * dcmx - xsm * (dcmy2 + dcmz2) + ysm * dcmxy + zsm * dcmxz;
        double pycm = 2.0 * dcmy - ysm * (dcmx2 + dcmz2) + zsm * dcmyz + xsm * dcmxy;
        double pzcm = 2.0 * dcmz - zsm * (dcmx2 + dcmy2) + xsm * dcmxz + ysm * dcmyz;

        // -----------------   TOTAL FIELD:    -----------------------------------
        bxsm = bxsm + (txcp + txcm) * term1[i] + (txcp - txcm) * term2[i] + (pxcp - pxcm) * term3[i] + (pxcp + pxcm) * term4[i];
        bysm = bysm + (tycp + tycm) * term1[i] + (tycp - tycm) * term2[i] + (pycp - pycm) * term3[i] + (pycp + pycm) * term4[i];
        bzsm = bzsm + (tzcp + tzcm) * term1[i] + (tzcp - tzcm) * term2[i] + (pzcp - pzcm) * term3[i] + (pzcp + pzcm) * term4[i];
    }

    // NOW CONVERT THE OBTAINED MAGNETIC FIELD VECTOR BACK FROM SM TO GSM (GSW) SYSTEM:
    // Convert from nT to Gauss
    Bfield[0] = (bxsm * cps + bzsm * sps) * 1e-5 * gauss;
    Bfield[1] = bysm * 1e-5 * gauss;
    Bfield[2] = (bzsm * cps - bxsm * sps) * 1e-5 * gauss;
}

void TA16::GetFieldValue(const G4double *Point, G4double *Bfield) const
{
    // Get the outer field in GSM coordinates
    double gsw_point[3];
    coord->geo_to_gsw(Point, gsw_point);
    G4cout << "GSW Point: " << gsw_point[0] / km << ", " << gsw_point[1] / km << ", " << gsw_point[2] / km << G4endl;
    double gsw_ta16_bfield[3];
    GetField_GSM(gsw_point, gsw_ta16_bfield);
    G4cout << "GSW TA16 field: " << gsw_ta16_bfield[0] / gauss << ", " << gsw_ta16_bfield[1] / gauss << ", " << gsw_ta16_bfield[2] / gauss << G4endl;
    double geo_ta16_bfield[3];
    coord->gsw_to_geo(gsw_ta16_bfield, geo_ta16_bfield);

    // Get the inner field in GEO coordinates
    inner_field->GetFieldValue(Point, Bfield);
    G4cout << "Inner field: " << Bfield[0] << ", " << Bfield[1] << ", " << Bfield[2] << G4endl;
    // Sum the inner and outer fields in GEO coordinates
    Bfield[0] += geo_ta16_bfield[0];
    Bfield[1] += geo_ta16_bfield[1];
    Bfield[2] += geo_ta16_bfield[2];

    G4cout << "Outer field: " << geo_ta16_bfield[0] << ", " << geo_ta16_bfield[1] << ", " << geo_ta16_bfield[2] << G4endl;
}