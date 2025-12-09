#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"
#include "G4EmStandardPhysics.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4StoppingPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"

class MagneticTransport : public G4VModularPhysicsList
{
public:
    MagneticTransport();
    ~MagneticTransport();

    void SetCuts() override;
};

class FTFP_BERT_Physics : public G4VModularPhysicsList
{
public:
    FTFP_BERT_Physics();
    ~FTFP_BERT_Physics();

    void SetCuts() override;
};

#endif