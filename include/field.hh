#ifndef FIELD_HH
#define FIELD_HH

#include "G4MagneticField.hh"
#include "G4SystemOfUnits.hh"

#include "utils.hh"

#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

class TabulatedEarthMagneticField : public G4MagneticField
{
public:
    TabulatedEarthMagneticField(const G4String datafile);
    ~TabulatedEarthMagneticField();

    void GetFieldValue(const G4double *Point, G4double *Bfield) const;

private:
    // Storage space for the table
    std::vector<std::vector<G4double>> BFieldTable;

    // Dimensions of the table
    G4int n_r, n_theta, n_phi;

    // physical limits of the defined region
    G4double r_min, r_max, theta_min, theta_max, phi_min, phi_max;

    // intervals of the grid
    G4double dr, dtheta, dphi;
};

class EarthMagneticField : public G4MagneticField
{
public:
    EarthMagneticField(const std::string datafile,
                       G4int year,
                       G4int month,
                       G4int day,
                       int N_MAX = -1);
    ~EarthMagneticField();

    void GetFieldValue(const G4double *Point, G4double *Bfield) const;

private:
    void LoadData(const std::string datafile);

    int N_MIN, N_MAX, NTIMES, SP_ORDER, N_STEPS;

    G4double year_frac;
    std::vector<std::vector<G4double>> g_interp;
    std::vector<std::vector<G4double>> h_interp;

    // Gauss coefficients
    std::vector<std::vector<std::vector<G4double>>> g;
    std::vector<std::vector<std::vector<G4double>>> h;
    std::vector<G4double> times;

    // Constants
    const G4double RE = 6371.2 * km; // Earth's reference radius
};

#endif