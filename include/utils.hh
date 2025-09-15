#ifndef UTILS_HH
#define UTILS_HH

#include "G4Types.hh"
#include "G4ios.hh"

#include <vector>

G4double trilinear_interp(G4double x_d, G4double y_d, G4double z_d,
                          G4double c000, G4double c100,
                          G4double c010, G4double c110,
                          G4double c001, G4double c101,
                          G4double c011, G4double c111);

void associated_legendre(G4int n, G4int m, G4double theta,
                         std::vector<std::vector<double>> &P,
                         std::vector<std::vector<double>> &dP);

bool is_leap_year(G4int year);

G4double datetime_to_yearfrac(G4int year, G4int month, G4int day);

#endif