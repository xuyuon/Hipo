#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    // Constructor implementation
    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("chargedgeantino");

    G4ThreeVector pos(0., 0., -20.*km);
    G4ThreeVector mom(0., 0., 1.); // create the particle moving in z direction

    fParticleGun->SetParticlePosition(pos); // Set the position of the particle
    fParticleGun->SetParticleMomentumDirection(mom); 
    fParticleGun->SetParticleMomentum(100.*GeV); // Set the momentum of the particle
    fParticleGun->SetParticleDefinition(particle); // Set the particle definition
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    // Destructor implementation
    delete fParticleGun; // Clean up the particle gun
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    fParticleGun->GeneratePrimaryVertex(anEvent); // Generate the primary vertex for the event
}