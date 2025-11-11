#include "physicsList.hh"

MagneticTransport::MagneticTransport()
    : G4VModularPhysicsList()
{
    SetVerboseLevel(0);

    // EM physics
    RegisterPhysics(new G4EmStandardPhysics());  

    // Step limiter
    G4StepLimiterPhysics* stepLimitPhys = new G4StepLimiterPhysics();
    stepLimitPhys->SetApplyToAll(true); // activates step limit for ALL particles
    RegisterPhysics(stepLimitPhys);
}

MagneticTransport::~MagneticTransport() {}

void MagneticTransport::SetCuts()
{
    G4VUserPhysicsList::SetCuts();
}