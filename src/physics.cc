#include "physics.hh"

MyPhysicsList::MyPhysicsList()
{
    // Constructor implementation
    RegisterPhysics (new G4EmStandardPhysics()); // Register electromagnetic interactions
    RegisterPhysics (new G4OpticalPhysics()); 
    RegisterPhysics (new G4DecayPhysics()); // particle decays
}

MyPhysicsList::~MyPhysicsList()
{
    // Destructor implementation
}

