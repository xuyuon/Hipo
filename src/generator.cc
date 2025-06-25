#include "generator.hh"

PrimaryGenerator::PrimaryGenerator(G4double h_ini, SamplingBase *energySpectrum)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager

    // Constructor implementation
    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");

    G4ThreeVector pos(0., 0., h_ini);
    G4ThreeVector mom(0., 0., -1.); // create the particle moving in z direction

    fParticleGun->SetParticlePosition(pos); 
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleDefinition(particle); 

    this->EnergySpectrum = energySpectrum; 
}

PrimaryGenerator::~PrimaryGenerator()
{
    // Destructor implementation
    delete fParticleGun; // Clean up the particle gun
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    G4double p_energy = EnergySpectrum->sample();
    analysisManager->FillNtupleDColumn(1, 0, p_energy / eV);
    analysisManager->AddNtupleRow(1);

    fParticleGun->SetParticleEnergy(p_energy);    // Set the momentum of the particle
    fParticleGun->GeneratePrimaryVertex(anEvent); // Generate the primary vertex for the event
}