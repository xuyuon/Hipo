#ifndef UTILS_HH
#define UTILS_HH

#include "G4Types.hh"


G4double trilinear_interp(G4double x_d, G4double y_d, G4double z_d,
                          G4double c000, G4double c100,
                          G4double c010, G4double c110,
                          G4double c001, G4double c101,
                          G4double c011, G4double c111);

#endif