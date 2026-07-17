#ifndef RUNMANAGER_HH
#define RUNMANAGER_HH

#include "G4MTRunManager.hh"
#include "G4MagneticField.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"
#include "FTFP_BERT.hh"

#include "action.hh"
#include "atmosphereModel.hh"
#include "densityModel.hh"
#include "field.hh"
#include "fileManager.hh"
#include "parser.hh"
#include "physicsList.hh"
#include "timer.hh"

#include <string>
#include <vector>

class G4EarthRunManager
{
public:
    G4EarthRunManager(G4String config_file);
    ~G4EarthRunManager();

    void run();

    void init_density_model();
    void init_field();
    void init_atmosphere();

private:
    void validate_config_schema();
    void validate_sampler(const std::string &label, std::vector<std::string> &errors, int depth = 0);
    std::vector<G4double> resolve_sensitive_layers();
    void write_run_metadata() const;

    std::string project_name;
    G4bool batch_mode;
    G4int nThreads;

    Timer *timer;
    Parser *parser;

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

    FileManager *fileManager;

    G4String config_file;
    std::string output_dir;
    std::string primaries_output_dir;
    std::vector<G4double> sensitive_layers;
};

#endif
