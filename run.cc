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

    LayerAtmosphereModel *atmophereModel = new LayerAtmosphereModel(
        new ConstTempBarometricModel(),
        40. * km,      // world_x
        40. * km,      // world_y
        10. * kg / m2, // Integrated density for every layer
        0. * km,       // altitude of the first layer
        100. * km,     // altitude of the last layer
        95. * km        // location of the sensitive layer
    );
    G4double epsilon = 1e-6 * m; 
    PrimaryGenerator *generator = new PrimaryGenerator(
        new PowerLaw(2.75, 10000. * MeV, 100000. * MeV), // energy spectrum sampling
        new ConstCoordinates(0., 0., atmophereModel->GetCeiling()/2. - epsilon),          // initial position sampling
        new ConstCoordinates(0., 0., -1.)                // nadir angle sampling
    );
    ActionInitialization *action = new ActionInitialization(generator);
    
    runManager->SetUserInitialization(atmophereModel);
    runManager->SetUserInitialization(action);
    

    // runManager->SetUserInitialization(new SphericalEarth(
    //     new ConstTempBarometricModel(),
    //     0. * km,      // h_min
    //     100. * km,    // h_max
    //     5. * km,      // h_sens
    //     10. * kg / m2 // Integrated density for every layer
    //     ));
    // runManager->SetUserInitialization(new ActionInitialization(
    //     new SphericalSurfaceSampling(
    //         new PowerLaw(2.75, 10000. * MeV, 100000. * MeV), // energy spectrum sampling
    //         new UniformSphericalSurface(6371.0 * km + 50.0 * km),          // initial position sampling
    //         new Scaled(new Sine(), 2.),              // nadir angle sampling
    //         new Uniform(0., 2 * PI) // azimuthal angle sampling
    //     )));


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