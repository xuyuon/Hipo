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

private:
    G4AnalysisManager *analysisManager;
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum;
    CoordinatesSamplingBase *pos_dist;
    SamplingBase *theta_dist;
    SamplingBase *phi_dist;

    std::string OutputDir;
};

class GenerateFromTable : public GeneratorBase
{
public:
    GenerateFromTable(
        G4String particleName, // name of the particle to be generated
        std::string data_file,
        std::string output_file = "output/primaries.csv", // output directory for analysis
        G4bool insert_finite_differences = false,
        G4double delta_E = 0.1 * GeV,
        G4double delta_angle = 0.5 * degree);
    ~GenerateFromTable();

    virtual void GeneratePrimaries(G4Event *);
    int get_n_events() const;

private:
    void init_table();
    void init_finite_differences();

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

    // Finite differences
    G4double delta_E;
    G4double delta_angle;

    // Directories
    std::string data_file;
    std::string output_file;
};

#endif