#include "densityModel.hh"


ConstTempBarometricModel::ConstTempBarometricModel(G4double temperature, G4double h_ref, G4double pressure_ref)
{
    this->temperature = temperature;
    this->h_ref = h_ref;
    this->pressure_ref = pressure_ref;
    R = 8.3144626181532; // ideal gas constant in J/(mol*K)
    g = 9.81;            // gravitational acceleration in m/s^2
    m_air = 0.028965;    // molar mass of dry air in kg/mol

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
    G4cout << "m_air = " << m_air << G4endl;
    G4cout << "R = " << R << G4endl;
    G4cout << "g = " << g << G4endl;
    G4cout << "temperature = " << temperature << G4endl;
    G4cout << "h_ref = " << h_ref << G4endl;
    G4cout << "pressure_ref = " << pressure_ref << G4endl;

    G4double density = m_air / R / temperature * pressure_ref * exp(-g * m_air * (altitude / m - h_ref) / (R * temperature)) * kg / m3;
    return density;
}