#ifndef DENSITYMODEL_HH
#define DENSITYMODEL_HH

#include "G4SystemOfUnits.hh"
#include "G4Types.hh"
#include "G4Exp.hh"
#include "G4Transportation.hh"

#include <vector>
#include <fstream>
#include <string>
#include <sstream>

class DensityModelBase
{
    /*
    DensityModelBase is a base class for air density models.
    It provides a list of boundaries along with the densities of
    the atmosphere between those boundaries.
    */
public:
    DensityModelBase() {}
    ~DensityModelBase() {}

    virtual std::vector<G4double> GetBoundaries() const = 0;
    virtual std::vector<G4double> GetDensities() const = 0;
};

class ConstTempBarometricModel : public DensityModelBase
{
public:
    ConstTempBarometricModel(
        G4double h_min,
        G4double h_max,
        G4double delta_h,
        G4double X_layer,
        G4double temperature = 300.,
        G4double h_ref = 0.,
        G4double pressure_ref = 101325.);
    ~ConstTempBarometricModel();

    virtual std::vector<G4double> GetBoundaries() const override;
    virtual std::vector<G4double> GetDensities() const override;

    G4double CalculateDensity(G4double altitude) const;

private:
    // Constants
    G4double temperature;  // Temperature in Kelvin
    G4double h_ref;        // Reference height in meters
    G4double pressure_ref; // Reference pressure in Pascals
    G4double R;            // Ideal gas constant in J/(mol*K)
    G4double g;            // Gravitational acceleration in m/s^2
    G4double m_air;        // Molar mass of dry air in kg/mol
    G4double beta;         // A constant defined as m_air * g / R / temperature

    // Geometry
    G4double h_min, h_max, delta_h; // Minimum height in meters

    // Integrated density
    G4double X_layer; // Integrated density for the air layer

    std::vector<G4double> boundaries;
    std::vector<G4double> densities;

    // Helper functions
    void InitBoundaries();
    G4double LayerCeiling(G4double altitude, G4double X) const;
    G4double MaxCeiling(G4double X) const;
};


class NRLMSIS_2 : public DensityModelBase
{
public:
    NRLMSIS_2(G4String data_file);
    ~NRLMSIS_2();

    virtual std::vector<G4double> GetBoundaries() const override;
    virtual std::vector<G4double> GetDensities() const override;

private:
    std::vector<G4double> boundaries;
    std::vector<G4double> densities;
};

#endif