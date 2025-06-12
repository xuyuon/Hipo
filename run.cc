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

int main(int argc, char** argv)
{   
    G4bool isMultiThreaded = true;
    G4bool isVisEnabled = false;

    // construct the run manager
    G4RunManager *runManager = nullptr;
    if (isMultiThreaded) {
        runManager = new G4MTRunManager();
    } else {
        runManager = new G4RunManager();
    }
    

    // set mandatory initialization classes
    G4VModularPhysicsList* physicsList = new FTFP_BERT();
    runManager->SetUserInitialization(physicsList);
    runManager->SetUserInitialization(new LayerAtmosphereModel()); 
    runManager->SetUserInitialization(new ActionInitialization());
    
    // initialize G4 kernel
    runManager->Initialize();

    G4UIExecutive *ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    G4UImanager *UImanager = G4UImanager::GetUIpointer();
    if (isVisEnabled)
    {   
        ui = new G4UIExecutive(argc, argv);
        G4VisManager *visManager = new G4VisExecutive();
        visManager->Initialize();

        
        UImanager->ApplyCommand("/vis/open OGL");
        UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 1 1");
        UImanager->ApplyCommand("/vis/drawVolume");
        UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true"); // Automatically refresh the view
        UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
        UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate"); // Accumulate the scene at the end of each event
        UImanager->ApplyCommand("/gun/particle proton");
        UImanager->ApplyCommand("/vis/ogl/set/displayListLimit 5000000");
        UImanager->ApplyCommand("/vis/scene/add/axes");

        ui->SessionStart();
    }
    else
    {
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }


    // job termination
    delete runManager;
    return 0;
}