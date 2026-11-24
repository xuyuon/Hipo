#include <gtest/gtest.h>

#include "field.hh"

// Test TA16::GetFieldValue
TEST(TestTA16, ValueCheck) {
    TA16 model = TA16(
        2008, 9, 15,
        0.92, // pdyn
        -28.7, // symhc
        0.2169, // xind
        -2.86, // by_imf
        -0.2309, // ps
        -534.2, 3.8, -0.1,
        "../test/input/test_TA16_RBF.par",
        "../test/input/test_field.shc",
        13
    );

    // Random test point in GEO coordinates
    G4double point_1[3] = {1.25 * 6371.2 * km, 3.12 * 6371.2 * km, -2.14 * 6371.2 * km};
    G4double Bfield_1[3];
    model.GetFieldValue(point_1, Bfield_1);

    G4double point_2[3] = {-2.5 * 6371.2 * km, 1.5 * 6371.2 * km, 4.0 * 6371.2 * km};
    G4double Bfield_2[3];
    model.GetFieldValue(point_2, Bfield_2);

    // Expected values (these values should be pre-calculated and verified)
    G4double expected_Bfield_1[3] = {319.20731288e-5 * gauss, 651.64916462e-5 * gauss, -61.38529835e-5 * gauss}; // Example expected values
    EXPECT_NEAR(Bfield_1[0], expected_Bfield_1[0], 1e-6 * gauss);
    EXPECT_NEAR(Bfield_1[1], expected_Bfield_1[1], 1e-6 * gauss);
    EXPECT_NEAR(Bfield_1[2], expected_Bfield_1[2], 1e-6 * gauss);

    G4double expected_Bfield_2[3] = {296.62294855e-5 * gauss, -202.38014868e-5 * gauss, -219.89795679e-5 * gauss}; // Example expected values
    EXPECT_NEAR(Bfield_2[0], expected_Bfield_2[0], 1e-6 * gauss);
    EXPECT_NEAR(Bfield_2[1], expected_Bfield_2[1], 1e-6 * gauss);
    EXPECT_NEAR(Bfield_2[2], expected_Bfield_2[2], 1e-6 * gauss);
}
