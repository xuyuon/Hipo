#include "steppingAction.hh"


SteppingAction::SteppingAction(EventAction* eventAction)
{
    E_min = 100 * MeV;
}


SteppingAction::~SteppingAction()
{

}


void SteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    G4String pName = track->GetParticleDefinition()->GetParticleName();


    if(pName=="nu_e"||pName=="anti_nu_e"||pName=="nu_mu"||pName=="anti_nu_mu"||pName=="nu_tau"||pName=="anti_nu_tau")
    {
        track->SetTrackStatus(fStopAndKill);
    }

    // Kill particles with energy below E_min
    G4double E_kinetic = track->GetKineticEnergy();
    if(E_kinetic < E_min && pName=="gamma")
    {
        track->SetTrackStatus(fStopAndKill);
    }

}
