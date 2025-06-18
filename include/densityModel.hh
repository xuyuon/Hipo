#ifndef DENSITYMODEL_HH
#define DENSITYMODEL_HH

#include "G4SystemOfUnits.hh"
#include "G4Types.hh"
#include "G4Exp.hh"
#include "G4Transportation.hh"


class DensityModelBase
{
public:
    DensityModelBase() {}
    ~DensityModelBase() {}


    virtual G4double CalculateDensity(G4double altitude) const = 0;
};


class ConstTempBarometricModel : public DensityModelBase
{
public:
    ConstTempBarometricModel(G4double temperature=300., G4double h_ref=0., G4double pressure_ref=101325.);
    ~ConstTempBarometricModel();

    virtual G4double CalculateDensity(G4double altitude) const override;

private:
    G4double temperature;  // Temperature in Kelvin
    G4double h_ref;        // Reference height in meters
    G4double pressure_ref; // Reference pressure in Pascals
    G4double R;          // Ideal gas constant in J/(mol*K)
    G4double g;          // Gravitational acceleration in m/s^2
    G4double m_air;      // Molar mass of dry air in kg/mol
};

#endif