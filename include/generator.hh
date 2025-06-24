#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"

#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh" // Include Geant4 units
#include "G4ParticleTable.hh"
#include "G4AnalysisManager.hh"

#include "sampling.hh"


class PrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGenerator();
    ~PrimaryGenerator();

    virtual void GeneratePrimaries(G4Event*);

private:
    G4AnalysisManager *analysisManager; 
    G4ParticleGun *fParticleGun; // Pointer to the particle gun

    SamplingBase *EnergySpectrum; 
};

#endif