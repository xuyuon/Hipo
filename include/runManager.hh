#ifndef RUNMANAGER_HH
#define RUNMANAGER_HH

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "FTFP_BERT.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4SystemOfUnits.hh"
#include "G4MagneticField.hh"

#include "parser.hh"
#include "timer.hh"
#include "densityModel.hh"
#include "atmosphereModel.hh"
#include "generator.hh"
#include "sampling.hh"
#include "action.hh"
#include "field.hh"

#include <filesystem>

class G4EarthRunManager
{
public:
    G4EarthRunManager(G4String config_file);
    ~G4EarthRunManager();

    void run();

    void init_density_model();
    void init_field();
    void init_atmosphere();
    void init_generator();

    SamplingBase *get_sampler(G4String sampler_name, G4String label) const;
    CoordinatesSamplingBase *get_coordinates_sampler(G4String sampler_name, G4String label) const;

private:
    // config
    std::string project_name;
    G4bool batch_mode;

    // utility classes
    Timer *timer;
    Parser *parser;

    // G4RunManager Components
    G4RunManager *runManager;
    G4VModularPhysicsList *physicsList;
    DensityModelBase *densityModel;
    AtmosphereModelBase *atmosphere;
    G4MagneticField *field;
    G4VUserPrimaryGeneratorAction *generator;
    ActionInitialization *action;
    G4UIExecutive *ui;
    G4UImanager *UImanager;
    G4VisManager *visManager;

    // directories
    G4String config_file;
    std::string output_dir;
};

#endif