#include <gtest/gtest.h>

#include "utils.hh"

// Test is_leap_year
TEST(TestIsLeapYear, ValueCheck) {
    // Case 1: not a leap year
    G4int year1 = 2025;
    EXPECT_FALSE(is_leap_year(year1));

    // Case 2: divisible by 4 but not by 100
    G4int year2 = 2024;
    EXPECT_TRUE(is_leap_year(year2));

    // Case 3: divisible by 100 but not by 400
    G4int year3 = 1900;
    EXPECT_FALSE(is_leap_year(year3));

    // Case 4: divisible by 400
    G4int year4 = 2000;
    EXPECT_TRUE(is_leap_year(year4));
}

// Test datetime_to_utc()
TEST(TestDatetimeToUTC, ValueCheck) {
    // Case 1: after epoch
    G4double utc = datetime_to_utc(2025, 12, 3);
    G4double expected_utc = 1764720000.0;
    EXPECT_DOUBLE_EQ(utc, expected_utc);

    // Case 2: on epoch
    utc = datetime_to_utc(1970, 1, 1);
    expected_utc = 0.0;
    EXPECT_DOUBLE_EQ(utc, expected_utc);

    // Case 3: before epoch
    utc = datetime_to_utc(1920, 2, 20);
    expected_utc = -1573603200.0;
    EXPECT_DOUBLE_EQ(utc, expected_utc);

    // Case 4: after epoch
    utc = datetime_to_utc(2111, 8, 3);
    expected_utc = 4468003200.0;
    EXPECT_DOUBLE_EQ(utc, expected_utc);

    // Case 5
    utc = datetime_to_utc(2008, 9, 15);
    expected_utc = 1221436800.0;
    EXPECT_DOUBLE_EQ(utc, expected_utc);

}