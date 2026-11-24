#ifndef FIELD_HH
#define FIELD_HH

#include "G4MagneticField.hh"
#include "G4SystemOfUnits.hh"

#include "utils.hh"
#include "coordinates.hh"

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

class IGRF : public G4MagneticField
{
public:
    IGRF(const std::string datafile,
         G4int year,
         G4int month,
         G4int day,
         int N_MAX = -1);
    ~IGRF();

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

class TA16 : public G4MagneticField
{
public:
    TA16(int year,
         int month,
         int day,
         G4double pdyn,
         G4double symhc,
         G4double xind,
         G4double by_imf,
         G4double ps,
         double vxgse,
         double vygse,
         double vzgse,
         std::string param_file,
         std::string igrf_datafile,
         int N_MAX = -1);
    ~TA16();

    // Get the magnetic field in GSM coordinates
    void GetField_GSM(const G4double *Point, G4double *Bfield) const;
    void GetFieldValue(const G4double *Point, G4double *Bfield) const;

private:
    // input parameters:
    //   PDYN                  - solar wind dynamic pressure in nPa
    //   <SymHc>               - corrected SymH index, to be calculated as <SymHc> = <0.8*SymH-13*sqrt(PDYN)>
    //                          where the angular brackets denote the sliding 30-min average, centered on the
    //                          current time moment
    //    <XIND>               - solar-wind-magnetosphere coupling index, based on Newell et al. [2007] function,
    //                          averaged over the previous 30-min trailing interval, see also the documentation
    //                          file TA16_Model_description.pdf for further details. Typical values of <XIND>
    //                          lie between 0 (quiet) and 2 (strongly disturbed)
    //   <IMF BY>              - azimuthal IMF By, in GSW (or GSM) coordinate system, in nanoTeslas, averaged over
    //                          the previous 30-min trailing interval
    //   PS                    - geodipole tilt angle (in radians)
    double pdyn, symhc, xind, by_imf, ps;
    int year, month, day;

    // FORWARDS RBF CENTER COORDINATES AND AUXILIARY PARAMS
    double xx[1296], yy[1296], zz[1296];
    double st[1296], rho[1296], zsp[1296], zcp[1296], rhbr[1296];

    double d = 4.0;
    double PI = 3.14159265359;
    double a_const[22] = {12.544, -0.194, 0.305, 0.0573, 2.178,
                          0.0571, -0.999, 16.473, 0.00152, 0.382, 0.0431, -0.00763,
                          -0.210, 0.0405, -4.430, -0.636, -2.600, 0.832, -5.328,
                          1.103, -0.907, 1.450};

    // parameters
    std::vector<double> a;

    // Construct RBF grid
    void ConstructRBFGrid();
    void PrecomputeGetFieldConstants();

    // Precompute constants used in GetField_GSM
    double fpd, symh, cps, sps, tps;
    double dxp_term[1296], dyp_term[1296], dzp_term[1296], dxm_term[1296], dym_term[1296], dzm_term[1296];
    double term1_plus_2[1296], term1_minus_2[1296], term4_plus_3[1296], term4_minus_3[1296];

    // inner field component
    IGRF *inner_field;

    // coordinate transformation
    GeocentricCoordinates *coord;
};

#endif
