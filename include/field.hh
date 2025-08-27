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

class EarthMagneticField : public G4MagneticField
{
public:
    EarthMagneticField(const G4String datafile);
    ~EarthMagneticField();

    void GetFieldValue(const G4double Point[4], G4double *Bfield) const;

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

#endif