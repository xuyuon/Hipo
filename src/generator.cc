#include "generator.hh"

PrimaryGenerator::PrimaryGenerator(SamplingBase *energySpectrum,
                                   CoordinatesSamplingBase *pos_dist,
                                   CoordinatesSamplingBase *dir_dist)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager

    // Constructor implementation
    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->dir_dist = dir_dist;
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun; // Clean up the particle gun
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    // Sample the particle energy
    G4double p_energy = EnergySpectrum->sample();
    analysisManager->FillNtupleDColumn(1, 0, p_energy / eV);
    analysisManager->AddNtupleRow(1);
    fParticleGun->SetParticleEnergy(p_energy); // Set the energy of the particle

    // Sample the initial position
    G4ThreeVector p_pos = pos_dist->sample();
    fParticleGun->SetParticlePosition(p_pos);

    // Sample the momentum direction
    G4ThreeVector p_dir = dir_dist->sample();
    fParticleGun->SetParticleMomentumDirection(p_dir);

    fParticleGun->GeneratePrimaryVertex(anEvent); // Generate the primary vertex for the event
}