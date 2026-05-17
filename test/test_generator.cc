#include <gtest/gtest.h>

#include "generator.hh"

// Test SphericalSurfaceSampling::nadir_angles_to_p_dir
// Test GenerateFromTable::nadir_angles_to_p_dir
TEST(TestNadirAnglesToPDir, ValueCheck) {
    // test case: pos_theta=pi/2, pos_phi=0, nadir_theta=pi/2, nadir_phi=0 -> x=0, y=0, z=1
    // test case: pos_theta=pi/2, pos_phi=pi/2, nadir_theta=pi/2, nadir_phi=0 -> x=0, y=0, z=1
    // test case: pos_theta=0, pos_phi=0, nadir_theta=0, nadir_phi=0 -> x=0, y=0, z=-1
    // test case: pos_theta=pi/2, pos_phi=0, nadir_theta=0, nadir_phi=0 -> x=-1, y=0, z=0
    // test case: pos_theta=pi/2, pos_phi=pi/2, nadir_theta=0, nadir_phi=0 -> x=0, y=-1, z=0
    // test case: pos_theta=pi/2, pos_phi=0, nadir_theta=pi/2, nadir_phi=pi/2 -> x=0, y=1, z=0
    // test case: pos_theta=pi/2, pos_phi=pi/2, nadir_theta=pi/2, nadir_phi=pi/2 -> x=-1, y=0, z=0

    G4ThreeVector p_dir1 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, 0, M_PI/2, 0);
    G4ThreeVector p_dir2 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, M_PI/2, M_PI/2, 0);
    G4ThreeVector p_dir3 = SphericalSurfaceSampling::nadir_angles_to_p_dir(0, 0, 0, 0);
    G4ThreeVector p_dir4 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, 0, 0, 0);
    G4ThreeVector p_dir5 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, M_PI/2, 0, 0);
    G4ThreeVector p_dir6 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, 0, M_PI/2, M_PI/2);
    G4ThreeVector p_dir7 = SphericalSurfaceSampling::nadir_angles_to_p_dir(M_PI/2, M_PI/2, M_PI/2, M_PI/2);

    G4ThreeVector p_dir_a = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, 0, M_PI/2, 0);
    G4ThreeVector p_dir_b = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, M_PI/2, M_PI/2, 0);
    G4ThreeVector p_dir_c = GenerateFromTable::nadir_angles_to_p_dir(0, 0, 0, 0);
    G4ThreeVector p_dir_d = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, 0, 0, 0);
    G4ThreeVector p_dir_e = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, M_PI/2, 0, 0);
    G4ThreeVector p_dir_f = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, 0, M_PI/2, M_PI/2);
    G4ThreeVector p_dir_g = GenerateFromTable::nadir_angles_to_p_dir(M_PI/2, M_PI/2, M_PI/2, M_PI/2);

    EXPECT_NEAR(p_dir1.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir1.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir1.z(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir2.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir2.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir2.z(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir3.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir3.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir3.z(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir4.x(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir4.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir4.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir5.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir5.y(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir5.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir6.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir6.y(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir6.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir7.x(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir7.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir7.z(), 0.0, 1e-6);

    EXPECT_NEAR(p_dir_a.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_a.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_a.z(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir_b.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_b.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_b.z(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir_c.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_c.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_c.z(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir_d.x(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir_d.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_d.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_e.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_e.y(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir_e.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_f.x(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_f.y(), 1.0, 1e-6);
    EXPECT_NEAR(p_dir_f.z(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_g.x(), -1.0, 1e-6);
    EXPECT_NEAR(p_dir_g.y(), 0.0, 1e-6);
    EXPECT_NEAR(p_dir_g.z(), 0.0, 1e-6);
}

