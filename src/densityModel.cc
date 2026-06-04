#include "densityModel.hh"

TabulatedDensity::TabulatedDensity(G4double r_min, G4double r_pause, G4double r_max,
                                   G4double delta_r1, G4double delta_r2,
                                   G4double delta_theta, G4double delta_phi, G4String data_file)
{
    this->r_min = r_min;
    this->r_pause = r_pause;
    this->r_max = r_max;
    this->delta_r1 = delta_r1;
    this->delta_r2 = delta_r2;
    this->delta_theta = delta_theta;
    this->delta_phi = delta_phi;
    this->data_file = data_file;

    R_earth = 6371.2 * km;

    InitBoundaries();
    LoadData();
}

TabulatedDensity::~TabulatedDensity()
{
}

void TabulatedDensity::InitBoundaries()
{
    // calculate the number of boundary points in each dimension
    G4int r_size = (r_pause - r_min) / delta_r1 + (r_max - r_pause) / delta_r2 + 1;
    G4int theta_size = 180 / (delta_theta / degree) + 1;
    G4int phi_size = 360 / (delta_phi / degree) + 1;

    // initialize h, glat, glong vectors
    r_boundaries.resize(r_size, -1.0);
    theta_boundaries.resize(theta_size, -1.0);
    phi_boundaries.resize(phi_size, -1.0);

    // number of bins in each dimension
    G4int nbin_r = r_size - 1;
    G4int nbin_theta = theta_size - 1;
    G4int nbin_phi = phi_size - 1;

    // resize all the 3D data vectors
    densities.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));
    h_percentage_mass.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));
    he_percentage_mass.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));
    o_percentage_mass.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));
    n_percentage_mass.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));
    ar_percentage_mass.resize(nbin_r, std::vector<std::vector<G4double>>(nbin_theta, std::vector<G4double>(nbin_phi, -1.0)));

    r_boundaries[0] = r_min;
    for (G4int i = 0; i < r_size - 1; i++)
    {
        if (i < (r_pause - r_min) / delta_r1)
            r_boundaries[i + 1] = r_boundaries[i] + delta_r1;
        else
            r_boundaries[i + 1] = r_boundaries[i] + delta_r2;
    }

    theta_boundaries[0] = 0.0 * degree;
    for (G4int i = 0; i < theta_size - 1; i++)
    {
        theta_boundaries[i + 1] = theta_boundaries[i] + delta_theta;
    }

    phi_boundaries[0] = 0.0 * degree;
    for (G4int i = 0; i < phi_size - 1; i++)
    {
        phi_boundaries[i + 1] = phi_boundaries[i] + delta_phi;
    }
}

void TabulatedDensity::LoadData()
{
    std::ifstream file(data_file);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open the density data file " << data_file << G4endl;
        return;
    }

    std::string line;

    // Read the first line to get the header information
    // we assume the data follow the format:
    // altitude(km), glat(deg), glong(deg), H_percentage_mass, He_percentage_mass, O_percentage_mass, N_percentage_mass, Ar_percentage_mass, density(g/cm3)
    std::getline(file, line);
    // TODO: check header format

    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string numstr;

        std::getline(ss, numstr, ',');
        // Find the index of the altitude in h_boundaries that is just lower or equal to numstr
        int r_index = upper_bound(r_boundaries.begin(), r_boundaries.end(), R_earth + std::stod(numstr) * km) - r_boundaries.begin() - 1;

        std::getline(ss, numstr, ',');
        // Find the index of the theta in theta_boundaries that is just lower or equal to numstr
        int theta_index = upper_bound(theta_boundaries.begin(), theta_boundaries.end(), (90.0 - std::stod(numstr)) * degree) - theta_boundaries.begin() - 1;

        std::getline(ss, numstr, ',');
        // Find the index of the phi in phi_boundaries that is just lower or equal to numstr
        int phi_index = upper_bound(phi_boundaries.begin(), phi_boundaries.end(), std::stod(numstr) * degree) - phi_boundaries.begin() - 1;

        std::getline(ss, numstr, ',');
        h_percentage_mass[r_index][theta_index][phi_index] = std::stod(numstr);

        std::getline(ss, numstr, ',');
        he_percentage_mass[r_index][theta_index][phi_index] = std::stod(numstr);

        std::getline(ss, numstr, ',');
        o_percentage_mass[r_index][theta_index][phi_index] = std::stod(numstr);

        std::getline(ss, numstr, ',');
        n_percentage_mass[r_index][theta_index][phi_index] = std::stod(numstr);

        std::getline(ss, numstr, ',');
        ar_percentage_mass[r_index][theta_index][phi_index] = std::stod(numstr);

        std::getline(ss, numstr, ',');
        densities[r_index][theta_index][phi_index] = std::stod(numstr) * g / cm3;
    }
    file.close();
}

// TODO: isALLFilled function