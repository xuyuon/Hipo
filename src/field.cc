#include "field.hh"

EarthMagneticField::EarthMagneticField(const G4String datafile)
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

EarthMagneticField::~EarthMagneticField()
{
    // Destructor
}

void EarthMagneticField::GetFieldValue(const G4double Point[4], G4double *Bfield) const
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