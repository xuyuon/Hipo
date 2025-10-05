#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"
#include "G4EmStandardPhysics.hh"
#include "G4StepLimiterPhysics.hh"

class MagneticTransport : public G4VModularPhysicsList
{
public:
    MagneticTransport();
    ~MagneticTransport();

    void SetCuts() override;
};

#endif