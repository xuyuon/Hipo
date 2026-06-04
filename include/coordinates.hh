#ifndef COORDINATES_HH
#define COORDINATES_HH

#include "G4SystemOfUnits.hh"
#include "utils.hh"

#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

class GeocentricCoordinates
{
public:
    GeocentricCoordinates(int year,
                          int month,
                          double day,
                          std::string field_datafile,
                          double vxgse,
                          double vygse,
                          double vzgse);
    ~GeocentricCoordinates();
    void gsm_to_gsw(const G4double* point_gsm, G4double* point_gsw) const;
    void gsw_to_gsm(const G4double* point_gsw, G4double* point_gsm) const;
    void geo_to_gsm(const G4double* point_geo, G4double* point_gsm) const;
    void gsm_to_geo(const G4double* point_gsm, G4double* point_geo) const;
    void geo_to_gsw(const G4double* point_geo, G4double* point_gsw) const;
    void gsw_to_geo(const G4double* point_gsw, G4double* point_geo) const;

private:
    // Private methods
    void LoadFieldData();
    void Sun();
    void InitTransformationMatrix();

    // Time information
    int year, month;
    double day;
    double ut; // universal time in seconds since epoch
    double year_frac; // year fraction

    // data file
    std::string field_datafile;
    int N_MIN, N_MAX, NTIMES, SP_ORDER, N_STEPS;
    std::vector<std::vector<G4double>> g_interp;
    std::vector<std::vector<G4double>> h_interp;

    // Observed solar wind velocity in GSE coordinates
    G4double vxgse, vygse, vzgse;  // in km/s

    // constants
    const double PI = 3.14159265359;

    // quantities necessary for coordinate transformations
    double gmst;       // Greenwich Mean Sidereal Time
    double slong;         // geocentric apparent ecliptic longitude, l
    double srasn, sdec;        // sun right ascension and declination
    double epsilon;   // mean obliquity of the ecliptic

    double cgst, sgst; 

    // elements of the matrix gsm to gsw
    double e11, e12, e13, e21, e22, e23, e31, e32, e33;
    // The elements of the matrix geo to gsm
    double a11, a12, a13, a21, a22, a23, a31, a32, a33;
    // The elements of the matrix geo to gsw
    double b11, b12, b13, b21, b22, b23, b31, b32, b33;
};

#endif