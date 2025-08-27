#include "utils.hh"

G4double trilinear_interp(G4double x_d, G4double y_d, G4double z_d,
                          G4double c000, G4double c100,
                          G4double c010, G4double c110,
                          G4double c001, G4double c101,
                          G4double c011, G4double c111)
{
    return c000 * (1 - x_d) * (1 - y_d) * (1 - z_d) +
           c100 * x_d * (1 - y_d) * (1 - z_d) +
           c010 * (1 - x_d) * y_d * (1 - z_d) +
           c110 * x_d * y_d * (1 - z_d) +
           c001 * (1 - x_d) * (1 - y_d) * z_d +
           c101 * x_d * (1 - y_d) * z_d +
           c011 * (1 - x_d) * y_d * z_d +
           c111 * x_d * y_d * z_d;
}