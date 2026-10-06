#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh" // Include Geant4 units
#include "G4ParticleTable.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"

#include "sampling.hh"
#include "outputManager.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>

class GeneratorBase : public G4VUserPrimaryGeneratorAction
{
public:
    GeneratorBase(){};
    virtual ~GeneratorBase(){};

    virtual void GeneratePrimaries(G4Event *) = 0;
    virtual int get_n_events() const = 0;
};

class SphericalSurfaceSampling : public GeneratorBase
{
public:
    SphericalSurfaceSampling(
        G4String particleName, // name of the particle to be generated
        SamplingBase *energySpectrum,
        CoordinatesSamplingBase *pos_dist,             // distribution of initial position
        SamplingBase *theta_dist,                      // distribution of theta angle (nadir angle)
        SamplingBase *phi_dist,                        // distribution of phi angle (azimuthal angle)
        OutputManager *outputManager = nullptr // pointer to the output manager
    );
    ~SphericalSurfaceSampling();

    virtual void GeneratePrimaries(G4Event *);
    int get_n_events() const { return -1; }
    static G4ThreeVector nadir_angles_to_p_dir(
        G4double pos_theta,
        G4double pos_phi,
        G4double nadir_theta,
        G4double nadir_phi);

private:
    G4AnalysisManager *analysisManager;
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum;
    CoordinatesSamplingBase *pos_dist;
    SamplingBase *theta_dist;
    SamplingBase *phi_dist;

    OutputManager *outputManager;
};

class GenerateFromTable : public GeneratorBase
{
public:
    GenerateFromTable(
        G4String particleName, // name of the particle to be generated
        std::string data_file,
        OutputManager *outputManager = nullptr
    );
    ~GenerateFromTable();

    virtual void GeneratePrimaries(G4Event *);
    int get_n_events() const;
    static G4ThreeVector nadir_angles_to_p_dir(
        G4double pos_theta,
        G4double pos_phi,
        G4double nadir_theta,
        G4double nadir_phi);

private:
    void init_table();

    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    //
    G4int n_threads;

    // Primaries parameters
    std::vector<G4double> energies;
    std::vector<G4double> pos_mag;
    std::vector<G4double> pos_phi;
    std::vector<G4double> pos_theta;
    std::vector<G4double> nadir_phi;
    std::vector<G4double> nadir_theta;

    // Directories
    std::string data_file;
    OutputManager *outputManager;
};

#endif
