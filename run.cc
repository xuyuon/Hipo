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

int main(int argc, char **argv)
{
    // Setting Up Parser
    Parser *parser = new Parser();
    G4String config_file = argv[1];
    parser->ReadConfig(config_file);

    // construct the run manager
    G4RunManager *runManager = nullptr;
    G4int nThreads = parser->GetIValue("nThreads");
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

    // DensityModelBase *densityModel = new NRLMSIS_2("msis2.0_density_profile.txt");
    // LayerAtmosphereModel *atmosphereModel = new LayerAtmosphereModel(
    //     densityModel,
    //     40. * km,      // world_x
    //     40. * km,      // world_y
    //     50. * km        // location of the sensitive layer
    // );
    // G4double epsilon = 1e-6 * m;
    // PrimaryGenerator *generator = new PrimaryGenerator(
    //     new PowerLaw(2.75, 10000. * MeV, 100000. * MeV), // energy spectrum sampling
    //     new ConstCoordinates(0., 0., atmosphereModel->GetCeiling()/2. - epsilon),          // initial position sampling
    //     new ConstCoordinates(0., 0., -1.)                // nadir angle sampling
    // );
    // ActionInitialization *action = new ActionInitialization(generator);

    G4double epsilon = 1e-6 * m;
    DensityModelBase *densityModel = new NRLMSIS_2(parser->GetSValue("DensityProfileFile"));
    SphericalEarth *atmosphereModel = new SphericalEarth(
        densityModel,                    // density model
        550. * km + epsilon,             // h_sens
        parser->GetSValue("SDOutputDir") // output directory for sensitive detector
    );
    SphericalSurfaceSampling *generator = new SphericalSurfaceSampling(
        "proton",                                                                         // particle name
        new LogUniform(parser->GetDValue("Emin") * MeV, parser->GetDValue("Emax") * MeV), // energy spectrum sampling
        new UniformSphericalSurface(atmosphereModel->GetCeiling() - epsilon),             // initial position sampling
        new Scaled(new Sine(), 2.),                                                       // nadir angle sampling
        new Uniform(0., 2. * PI),                                                         // azimuthal angle sampling
        parser->GetSValue("GenOutputDir")                                                 // output directory for analysis
    );
    ActionInitialization *action = new ActionInitialization(generator);

    runManager->SetUserInitialization(atmosphereModel);
    runManager->SetUserInitialization(action);

    // initialize G4 kernel
    runManager->Initialize();

    G4UIExecutive *ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    if (parser->GetBValue("batch_mode"))
    {
        // batch mode
        G4String command = "/control/execute ";
        G4String fileName = parser->GetSValue("macro_file");
        UImanager->ApplyCommand(command + fileName);
    }
    else
    {
        // interactive mode
        ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // job termination
    delete runManager;
    return 0;
}