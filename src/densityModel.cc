#include "densityModel.hh"

ConstTempBarometricModel::ConstTempBarometricModel(
    G4double h_min,
    G4double h_max,
    G4double delta_h,
    G4double X_layer,
    G4double temperature,
    G4double h_ref,
    G4double pressure_ref)
{
    this->h_min = h_min;
    this->h_max = h_max;
    this->delta_h = delta_h;
    this->X_layer = X_layer;
    this->temperature = temperature;
    this->h_ref = h_ref;
    this->pressure_ref = pressure_ref;

    R = 8.3144626181532; // ideal gas constant in J/(mol*K)
    g = 9.81;            // gravitational acceleration in m/s^2
    m_air = 0.028965;    // molar mass of dry air in kg/mol

    beta = m_air * g / R / temperature;

    InitBoundaries();
}

ConstTempBarometricModel::~ConstTempBarometricModel()
{
}

void ConstTempBarometricModel::InitBoundaries()
{
    boundaries.clear();
    densities.clear();

    boundaries = {h_min}; // Start with the ground level
    G4double max_ceiling = MaxCeiling(X_layer);

    // filling the space up to maximum ceiling height with fixed integrated density
    while (boundaries.back() < max_ceiling)
    {
        G4double nextBoundary = LayerCeiling(boundaries.back(), X_layer);
        if (std::isnan(nextBoundary))
        {
            G4cout << "Error: nextBoundary is NaN at altitude " << boundaries.back() / km << " km" << G4endl;
            break; // Stop if we encounter a NaN value
        }
        if (nextBoundary > h_max)
            break; // Stop if we exceed the maximum altitude
        boundaries.push_back(nextBoundary);
    }

    // filling the space beyond maximum ceiling height
    if (h_max > max_ceiling)
    {
        G4cout << "h_max exceeds the maximum ceiling height, filling the space "
                  "beyond maximum ceiling height with layer of thickness "
               << delta_h / km << "km" << G4endl;

        while (boundaries.back() < h_max)
        {
            G4double nextBoundary = boundaries.back() + delta_h;
            if (nextBoundary > h_max)
                break; // Stop if we exceed the maximum altitude
            boundaries.push_back(nextBoundary);
        }
    }

    // filling in the densities
    G4int n_boundaries = boundaries.size();
    for (int i = 0; i < n_boundaries; i++)
    {
        densities.push_back(CalculateDensity(boundaries[i]));
    }

    // filling in the percentage mass of each element
    h_percentage_mass.assign(n_boundaries, 0.0);
    he_percentage_mass.assign(n_boundaries, 0.0);
    n_percentage_mass.assign(n_boundaries, 0.78);
    o_percentage_mass.assign(n_boundaries, 0.22);
    ar_percentage_mass.assign(n_boundaries, 0.0);
}

std::vector<G4double> ConstTempBarometricModel::GetBoundaries() const
{
    return boundaries;
}

std::vector<G4double> ConstTempBarometricModel::GetDensities() const
{
    return densities;
}

std::vector<G4double> ConstTempBarometricModel::GetHPercentageMass() const
{
    return h_percentage_mass;
}

std::vector<G4double> ConstTempBarometricModel::GetHePercentageMass() const
{
    return he_percentage_mass;
}

std::vector<G4double> ConstTempBarometricModel::GetNPercentageMass() const
{
    return n_percentage_mass;
}

std::vector<G4double> ConstTempBarometricModel::GetOPercentageMass() const
{
    return o_percentage_mass;
}

std::vector<G4double> ConstTempBarometricModel::GetArPercentageMass() const
{
    return ar_percentage_mass;
}

G4double ConstTempBarometricModel::CalculateDensity(G4double altitude) const
{
    /*
    Calculates the density of air (in newtons per square meter) at a given altitude (in m)
    using the barometric formula.
    */
    G4double density = m_air / R / temperature * pressure_ref * exp(-g * m_air * (altitude / m - h_ref) / (R * temperature)) * kg / m3;
    return density;
}

G4double ConstTempBarometricModel::LayerCeiling(G4double altitude, G4double X) const
{
    /*
    Given an altitude, calculates the height of the ceiling above that altitude
    so that the integrated density from the altitude to the ceiling is equal to X.
    */
    G4double ceiling = -1 / beta * log(exp(-beta * altitude / m) - g * (X / kg * m2) / pressure_ref / exp(-beta * h_ref / m));
    return ceiling * m;
}

G4double ConstTempBarometricModel::MaxCeiling(G4double X) const
{
    /*
    Calculates the maximum ceiling for a given integrated density X.
    Inputting any altitude above this ceiling will return NaN in CalculateCeiling.
    */
    G4double maxCeiling = -1 / beta * log(g * (X / kg * m2) / pressure_ref / exp(beta * h_ref / m));
    return maxCeiling * m;
}

NRLMSIS_2::NRLMSIS_2(G4String data_file)
{
    std::ifstream file(data_file);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << data_file << G4endl;
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

        std::getline(ss, numstr, ',');
        boundaries.push_back(std::stod(numstr) * km);
        G4cout << "Altitude: " << boundaries.back() / km << " km" << G4endl;

        std::getline(ss, numstr, ',');
        h_percentage_mass.push_back(std::stod(numstr));
        G4cout << "H percentage mass: " << h_percentage_mass.back() << G4endl;

        std::getline(ss, numstr, ',');
        he_percentage_mass.push_back(std::stod(numstr));
        G4cout << "He percentage mass: " << he_percentage_mass.back() << G4endl;

        std::getline(ss, numstr, ',');
        n_percentage_mass.push_back(std::stod(numstr));
        G4cout << "N percentage mass: " << n_percentage_mass.back() << G4endl;

        std::getline(ss, numstr, ',');
        o_percentage_mass.push_back(std::stod(numstr));
        G4cout << "O percentage mass: " << o_percentage_mass.back() << G4endl;

        std::getline(ss, numstr, ',');
        ar_percentage_mass.push_back(std::stod(numstr));
        G4cout << "Ar percentage mass: " << ar_percentage_mass.back() << G4endl;

        std::getline(ss, numstr, ',');
        densities.push_back(std::stod(numstr) * g / cm3);
        G4cout << "Density: " << densities.back() / g * cm3 << " g/cm^3" << G4endl;
    }
}

NRLMSIS_2::~NRLMSIS_2()
{
}

std::vector<G4double> NRLMSIS_2::GetBoundaries() const
{
    return boundaries;
}

std::vector<G4double> NRLMSIS_2::GetDensities() const
{
    return densities;
}

std::vector<G4double> NRLMSIS_2::GetHPercentageMass() const
{
    return h_percentage_mass;
}

std::vector<G4double> NRLMSIS_2::GetHePercentageMass() const
{
    return he_percentage_mass;
}

std::vector<G4double> NRLMSIS_2::GetNPercentageMass() const
{
    return n_percentage_mass;
}

std::vector<G4double> NRLMSIS_2::GetOPercentageMass() const
{
    return o_percentage_mass;
}

std::vector<G4double> NRLMSIS_2::GetArPercentageMass() const
{
    return ar_percentage_mass;
}
