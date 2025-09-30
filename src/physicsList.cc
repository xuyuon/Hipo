# include "physicsList.hh"

MagneticTransport::MagneticTransport()
    : G4VModularPhysicsList()
{
    SetVerboseLevel(0);

    // EM physics
    RegisterPhysics(new G4EmStandardPhysics());  
}

MagneticTransport::~MagneticTransport() {}

void MagneticTransport::SetCuts()
{
    G4VUserPhysicsList::SetCuts();
}