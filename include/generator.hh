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


class PrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGenerator(
        SamplingBase *energySpectrum,      // pointer to the energy spectrum sampling object
        CoordinatesSamplingBase *pos_dist, // distribution of initial position
        CoordinatesSamplingBase *dir_dist  // distribution of direction of initial momentum
    );
    ~PrimaryGenerator();

    virtual void GeneratePrimaries(G4Event *);

private:
    G4AnalysisManager *analysisManager;
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum;
    CoordinatesSamplingBase *pos_dist;
    CoordinatesSamplingBase *dir_dist;
};

class SphericalSurfaceSampling : public G4VUserPrimaryGeneratorAction
{
public:
    SphericalSurfaceSampling(
        G4String particleName, // name of the particle to be generated
        SamplingBase *energySpectrum,
        CoordinatesSamplingBase *pos_dist,            // distribution of initial position
        SamplingBase *theta_dist,                     // distribution of theta angle (nadir angle)
        SamplingBase *phi_dist,                       // distribution of phi angle (azimuthal angle)
        std::string OutputDir = "output/primaries.csv" // output directory for analysis
    );
    ~SphericalSurfaceSampling();

    virtual void GeneratePrimaries(G4Event *);

private:
    G4AnalysisManager *analysisManager;
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum;
    CoordinatesSamplingBase *pos_dist;
    SamplingBase *theta_dist;
    SamplingBase *phi_dist;

    std::string OutputDir;
};

class FiniteDifferenceSampling : public G4VUserPrimaryGeneratorAction
{
public:
    FiniteDifferenceSampling(
        G4String particleName, // name of the particle to be generated
        std::string data_file,
        std::string output_file = "output/primaries_fd.csv" // output directory for analysis
    );
    ~FiniteDifferenceSampling();

    virtual void GeneratePrimaries(G4Event *);
private:
    void init_primaries_params();

    // 
    G4int event_count;

    // Directories
    std::string data_file;
    std::string output_file;
};

#endif