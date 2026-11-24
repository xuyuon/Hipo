#include <gtest/gtest.h>

#include "coordinates.hh"

// Test GeocentricCoordinates::geo_to_gsw
TEST(TestGeoToGsw, ValueCheck) {
    // assume the program is run from the build directory
    std::string field_datafile = "../test/input/test_field.shc";

    // some random test points in km
    G4double x1 = 3.24 * km;
    G4double y1 = -2.17 * km;
    G4double z1 = 1.05 * km;
    G4double x2 = -1.24 * km;
    G4double y2 = 4.17 * km;
    G4double z2 = -3.05 * km;

    // Case 1: 15 September 2008 00:00
    GeocentricCoordinates geoCoord1(2008, 9, 15, field_datafile, -534.2, 3.8, 0.1);
    G4double point_geo1[3] = {x1, y1, z1};
    G4double point_gsw1[3];
    geoCoord1.geo_to_gsw(point_geo1, point_gsw1);
    EXPECT_NEAR(point_gsw1[0] / km, -3.2362973167274296, 1e-4);
    EXPECT_NEAR(point_gsw1[1] / km, 1.8498838311799084, 1e-4);
    EXPECT_NEAR(point_gsw1[2] / km, 1.5534830185367312, 1e-4);
    G4double point_geo2[3] = {x2, y2, z2};
    G4double point_gsw2[3];
    geoCoord1.geo_to_gsw(point_geo2, point_gsw2);
    EXPECT_NEAR(point_gsw2[0] / km, 1.185995439571947, 1e-4);
    EXPECT_NEAR(point_gsw2[1] / km, -3.5646779634962633, 1e-4);
    EXPECT_NEAR(point_gsw2[2] / km, -3.7570581355468384, 1e-4);

}

// Test GeocentricCoordinates::gsw_to_geo
TEST(TestGswToGeo, ValueCheck) {
    // assume the program is run from the build directory
    std::string field_datafile = "../test/input/test_field.shc";

    // some random test points in km
    G4double x1 = -3.2362973167274296 * km;
    G4double y1 = 1.8498838311799084 * km;
    G4double z1 = 1.5534830185367312 * km;
    G4double x2 = 1.185995439571947 * km;
    G4double y2 = -3.5646779634962633 * km;
    G4double z2 = -3.7570581355468384 * km;

    // Case 1: 15 September 2008 00:00
    GeocentricCoordinates geoCoord1(2008, 9, 15, field_datafile, -534.2, 3.8, 0.1);
    G4double point_gsw1[3] = {x1, y1, z1};
    G4double point_geo1[3];
    geoCoord1.gsw_to_geo(point_gsw1, point_geo1);
    EXPECT_NEAR(point_geo1[0] / km, 3.24, 1e-4);
    EXPECT_NEAR(point_geo1[1] / km, -2.17, 1e-4);
    EXPECT_NEAR(point_geo1[2] / km, 1.05, 1e-4);
    G4double point_gsw2[3] = {x2, y2, z2};
    G4double point_geo2[3];
    geoCoord1.gsw_to_geo(point_gsw2, point_geo2);
    EXPECT_NEAR(point_geo2[0] / km, -1.24, 1e-4);
    EXPECT_NEAR(point_geo2[1] / km, 4.17, 1e-4);
    EXPECT_NEAR(point_geo2[2] / km, -3.05, 1e-4);
}
