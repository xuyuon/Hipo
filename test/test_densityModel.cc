#include <gtest/gtest.h>

#include "densityModel.hh"

// Test GeocentricCoordinates::geo_to_gsw
TEST(TestTabulatedDensity, ValueCheck)
{
    TabulatedDensity densityModel(
        6371.2 * km,                             // r_min
        6371.2 * km + 100.0 * km,                // r_pause
        6371.2 * km + 500.0 * km,                // r_max
        1.0 * km,                                // delta_r1
        10.0 * km,                               // delta_r2
        10.0 * deg,                              // delta_theta
        20.0 * deg,                              // delta_phi
        "../test/input/test_density_profile.txt" // data_file
    );

    // Check Maximum value of r boundaries
    EXPECT_DOUBLE_EQ(densityModel.Get_r(densityModel.Get_r_boundaries_size() - 1),  6371.2 * km + 500.0 * km);

    // Check some known values from the test_density_profile.txt
    // Case 1: r = 6371.2 km + 0.5 km (0.5 km altitude), theta = 175 deg (-85 deg glat), phi = 0 deg (0 deg glong)
    EXPECT_DOUBLE_EQ(densityModel.GetDensities(0, 17, 0), 0.001313 * g / cm3);
    EXPECT_DOUBLE_EQ(densityModel.GetHPercentageMass(0, 17, 0), 1.2746348006029173e-58);
    EXPECT_DOUBLE_EQ(densityModel.GetHePercentageMass(0, 17, 0), 7.192265071230317e-07);
    EXPECT_DOUBLE_EQ(densityModel.GetOPercentageMass(0, 17, 0), 0.2315753635502017);
    EXPECT_DOUBLE_EQ(densityModel.GetNPercentageMass(0, 17, 0), 0.7556957315433661);
    EXPECT_DOUBLE_EQ(densityModel.GetArPercentageMass(0, 17, 0), 0.012883377855707038);

    // Case 2: r = 6371.2 km + 88.5 km (88.5 km altitude), theta = 135.0 deg (-45.0 deg glat), phi = 170 deg (170 deg glong)
    EXPECT_DOUBLE_EQ(densityModel.GetDensities(88, 13, 8), 3.22e-09 * g / cm3);
    // check the lower boundary value
    EXPECT_DOUBLE_EQ(densityModel.Get_r(88),  88 * km + 6371.2 * km); 
    EXPECT_DOUBLE_EQ(densityModel.Get_theta(13), 130.0 * deg); 
    EXPECT_DOUBLE_EQ(densityModel.Get_phi(8), 160.0 * deg);

    // Case 3: r = 6371.2 km + 475.0 km (475 km altitude), theta = 45.0 deg (45 deg glat), phi = 130 deg (130 deg glong)
    EXPECT_DOUBLE_EQ(densityModel.GetDensities(137, 4, 6), 1.525e-16 * g / cm3);
    // check the lower boundary value
    EXPECT_DOUBLE_EQ(densityModel.Get_r(137),  470 * km + 6371.2 * km);
    EXPECT_DOUBLE_EQ(densityModel.Get_theta(4), 40.0 * deg);
    EXPECT_DOUBLE_EQ(densityModel.Get_phi(6), 120.0 * deg);
}
