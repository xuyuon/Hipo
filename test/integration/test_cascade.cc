#include <gtest/gtest.h>

#include "G4Types.hh"

#include "runManager.hh"


TEST(TestCascade, IntegrationTest) {
    G4String config_file = "../test/input/test_cascade.config.txt";

    G4EarthRunManager *manager = new G4EarthRunManager(config_file);
    manager->run();

    delete manager;
}
