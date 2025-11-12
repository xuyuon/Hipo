#include <iostream>

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4SystemOfUnits.hh"
#include "FTFP_BERT.hh"

#include "action.hh"
#include "atmosphereModel.hh"
#include "densityModel.hh"
#include "generator.hh"
#include "sampling.hh"
#include "parser.hh"
#include "timer.hh"

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