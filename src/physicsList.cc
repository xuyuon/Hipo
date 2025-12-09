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

FTFP_BERT_Physics::FTFP_BERT_Physics()
    : G4VModularPhysicsList()
{
    SetVerboseLevel(0);
    // FTFP_BERT
    // EM physics
    RegisterPhysics(new G4EmStandardPhysics());  

    // Synchroton Radiation & GN Physics
    RegisterPhysics( new G4EmExtraPhysics() );

    // Decay physics
    RegisterPhysics(new G4DecayPhysics());
    
    // Hadronic scattering physics
    RegisterPhysics(new G4HadronElasticPhysics());

    // Hadron physics
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT());

    // Stopping Physics
    RegisterPhysics( new G4StoppingPhysics() );

    // Ion Physics
    RegisterPhysics( new G4IonPhysics());
    
    // Neutron tracking cut
    RegisterPhysics( new G4NeutronTrackingCut());

    // Step limiter
    G4StepLimiterPhysics* stepLimitPhys = new G4StepLimiterPhysics();
    stepLimitPhys->SetApplyToAll(true); // activates step limit for ALL particles
    RegisterPhysics(stepLimitPhys);
}

FTFP_BERT_Physics::~FTFP_BERT_Physics() {}

void FTFP_BERT_Physics::SetCuts()
{
    G4VUserPhysicsList::SetCuts();
}