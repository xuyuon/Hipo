#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh" // Include Geant4 units
#include "G4ParticleTable.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"

#include "sampling.hh"

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGenerator(
        SamplingBase *energySpectrum,      // pointer to the energy spectrum sampling object
        CoordinatesSamplingBase *pos_dist, // distribution of initial position
        CoordinatesSamplingBase *dir_dist // distribution of direction of initial momentum
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
        SamplingBase *energySpectrum,
        CoordinatesSamplingBase *pos_dist, // distribution of initial position
        SamplingBase *theta_dist, // distribution of theta angle (nadir angle)
        SamplingBase *phi_dist,  // distribution of phi angle (azimuthal angle)
        std::string GenOutputDir = "output/generator" // output directory for analysis
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

    std::string GenOutputDir, GenOutputFile; 
};

#endif