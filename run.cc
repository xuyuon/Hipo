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

int main(int argc, char **argv)
{
    G4int nThreads = 4; // nThreads=1 for single-threaded mode, nThreads>1 for multi-threaded mode

    // construct the run manager
    G4RunManager *runManager = nullptr;
    if (nThreads > 1)
    {
        runManager = new G4MTRunManager();
        runManager->SetNumberOfThreads(nThreads);
    }
    else
    {
        runManager = new G4RunManager();
    }

    // set mandatory initialization classes
    G4VModularPhysicsList *physicsList = new FTFP_BERT();
    runManager->SetUserInitialization(physicsList);
    runManager->SetUserInitialization(new LayerAtmosphereModel(
        new ConstTempBarometricModel(),
        40. * km, // world_x
        40. * km, // world_y
        10. * kg / m2, // Integrated density for every layer
        0. * km, // altitude of the first layer
        100. * km, // altitude of the last layer
        5. * km // location of the sensitive layer
        ));
    runManager->SetUserInitialization(new ActionInitialization());

    // initialize G4 kernel
    runManager->Initialize();

    G4UIExecutive *ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    if (argc != 1)
    {
        // batch mode
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    else
    {
        // interactive mode
        ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // job termination
    delete runManager;
    return 0;
}