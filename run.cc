#include <iostream>

#include "G4Types.hh"

#include "runManager.hh"

int main(int argc, char **argv)
{
    G4String config_file = argv[1];

    G4EarthRunManager *manager = new G4EarthRunManager(config_file);
    manager->run();

    delete manager;
    G4cout << "Run Completed" << G4endl;

    return 0;
}
