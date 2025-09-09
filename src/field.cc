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

EarthMagneticField::EarthMagneticField(const std::string datafile,
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

EarthMagneticField::~EarthMagneticField()
{
    // Destructor
}

void EarthMagneticField::LoadData(const std::string datafile)
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

    // 5. Fine t1 (the time point just before the input year) and
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

void EarthMagneticField::GetFieldValue(const G4double *Point, G4double *Bfield) const
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

    // Convert from spherical to Cartesian coordinates
    Bfield[0] = sin(theta) * cos(phi) * Br + cos(theta) * cos(phi) * Btheta - sin(phi) * Bphi; // Bx
    Bfield[1] = sin(theta) * sin(phi) * Br + cos(theta) * sin(phi) * Btheta + cos(phi) * Bphi; // By
    Bfield[2] = cos(theta) * Br - sin(theta) * Btheta;                                         // Bz
}