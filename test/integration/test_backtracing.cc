#include <gtest/gtest.h>

#include "G4Types.hh"

#include "runManager.hh"


TEST(TestBacktracing, IntegrationTest) {
    G4String config_file = "../test/input/test_backtracing.config.txt";

    G4EarthRunManager *manager = new G4EarthRunManager(config_file);
    manager->run();

    delete manager;
}
