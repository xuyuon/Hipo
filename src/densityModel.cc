#include "densityModel.hh"


ConstTempBarometricModel::ConstTempBarometricModel(G4double temperature, G4double h_ref, G4double pressure_ref)
{
    this->temperature = temperature;
    this->h_ref = h_ref;
    this->pressure_ref = pressure_ref;
    R = 8.3144626181532; // ideal gas constant in J/(mol*K)
    g = 9.81;            // gravitational acceleration in m/s^2
    m_air = 0.028965;    // molar mass of dry air in kg/mol

    beta = m_air * g / R / temperature;
}

ConstTempBarometricModel::~ConstTempBarometricModel()
{
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


G4double ConstTempBarometricModel::CalculateCeiling(G4double altitude, G4double X) const
{
    /*
    Given an altitude, calculates the height of the ceiling above that altitude
    so that the integrated density from the altitude to the ceiling is equal to X.
    */
    G4double ceiling = - 1 / beta * log(exp(- beta * altitude / m) - g * (X / kg * m2) / pressure_ref / exp(- beta * h_ref / m));
    return ceiling * m;
}


G4double ConstTempBarometricModel::CalculateMaxCeiling(G4double X) const
{
    /*
    Calculates the maximum ceiling for a given integrated density X.
    Inputting any altitude above this ceiling will return NaN in CalculateCeiling.
    */
    G4double maxCeiling = - 1 / beta * log(g * (X / kg * m2) / pressure_ref / exp(beta * h_ref / m));
    return maxCeiling * m;
}