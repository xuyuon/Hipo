#include <gtest/gtest.h>

#include "sensitiveDetector.hh"

// Test SphericalShellSD::p_mom_to_nadir_angles
TEST(TestPMomToNadirAngles, ValueCheck) {
    // Test case 1: position at (1 km, 1 km, 0 km) and momentum (0, 0, -1)
    //  => expected nadir_theta = 90 deg, nadir_phi = 0 deg
    // Test case 2: position at (1 km, 1 km, 0 km) and momentum (1/sqrt(2), -1/sqrt(2), 0)
    //  => expected nadir_theta = 90 deg, nadir_phi = 90 deg
    // Test case 3: position at (1 km, 1 km, 0 km) and momentum (1/sqrt(3), 1/sqrt(3), -1/sqrt(3))
    //  => expected nadir_theta = arctan(1/sqrt(2)) rad, nadir_phi = 0 deg
    G4ThreeVector *p_pos1 = new G4ThreeVector(1.0 * km, 1.0 * km, 0.0 * km); 
    G4ThreeVector *p_mom1 = new G4ThreeVector(0.0, 0.0, -1.0);      
    G4ThreeVector *p_pos2 = new G4ThreeVector(1.0 * km, 1.0 * km, 0.0 * km);
    G4ThreeVector *p_mom2 = new G4ThreeVector(1.0 / sqrt(2.0), -1.0 / sqrt(2.0), 0.0);     
    G4ThreeVector *p_pos3 = new G4ThreeVector(1.0 * km, 1.0 * km, 0.0 * km);
    G4ThreeVector *p_mom3 = new G4ThreeVector(1.0 / sqrt(3.0), 1.0 / sqrt(3.0), -1.0 / sqrt(3.0));


    G4double nadir_angles1[2];
    G4double nadir_angles2[2];
    G4double nadir_angles3[2];

    SphericalShellSD::p_mom_to_nadir_angles(p_pos1, p_mom1, nadir_angles1);
    SphericalShellSD::p_mom_to_nadir_angles(p_pos2, p_mom2, nadir_angles2);
    SphericalShellSD::p_mom_to_nadir_angles(p_pos3, p_mom3, nadir_angles3);

    
    EXPECT_NEAR(nadir_angles1[0] / degree, 90.0, 1e-4);
    EXPECT_NEAR(nadir_angles1[1] / degree, 0.0, 1e-4);
    EXPECT_NEAR(nadir_angles2[0] / degree, 90.0, 1e-4);
    EXPECT_NEAR(nadir_angles2[1] / degree, 90.0, 1e-4);
    EXPECT_NEAR(nadir_angles3[0] / radian, atan(1.0/sqrt(2.0)), 1e-4);
    EXPECT_NEAR(nadir_angles3[1] / degree, 0.0, 1e-4);

}