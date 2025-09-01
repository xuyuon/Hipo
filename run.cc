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


    return 0;
}