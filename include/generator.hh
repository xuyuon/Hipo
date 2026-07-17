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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <unordered_set>
#include <vector>

class GeneratorBase : public G4VUserPrimaryGeneratorAction
{
public:
    GeneratorBase(){};
    virtual ~GeneratorBase(){};

    virtual void GeneratePrimaries(G4Event *) = 0;
    virtual int get_n_events() const = 0;
};

class PrimaryGenerator : public GeneratorBase
{
public:
    PrimaryGenerator(
        SamplingBase *energySpectrum,      // pointer to the energy spectrum sampling object
        CoordinatesSamplingBase *pos_dist, // distribution of initial position
        CoordinatesSamplingBase *dir_dist  // distribution of direction of initial momentum
    );
    ~PrimaryGenerator();

    virtual void GeneratePrimaries(G4Event *);
    int get_n_events() const { return -1; }

private:
    void ensure_output_file(int thread_id) const;

    G4AnalysisManager *analysisManager;
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum;
    CoordinatesSamplingBase *pos_dist;
    CoordinatesSamplingBase *dir_dist;
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
        std::string OutputDir = "output/primaries.csv" // output directory for analysis
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

    std::string OutputDir;
    mutable std::mutex output_init_mutex;
    mutable std::unordered_set<int> initialized_output_threads;
};

class GenerateFromTable : public GeneratorBase
{
public:
    GenerateFromTable(
        G4String particleName, // name of the particle to be generated
        std::string data_file,
        std::string output_file = "output/primaries.csv" // output directory for analysis
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
    void ensure_output_file(int thread_id) const;

    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    // Primaries parameters
    std::vector<G4double> energies;
    std::vector<G4double> pos_mag;
    std::vector<G4double> pos_phi;
    std::vector<G4double> pos_theta;
    std::vector<G4double> nadir_phi;
    std::vector<G4double> nadir_theta;

    // Directories
    std::string data_file;
    std::string output_file;
    mutable std::mutex output_init_mutex;
    mutable std::unordered_set<int> initialized_output_threads;
};

#endif