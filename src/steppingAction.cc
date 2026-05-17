#include "steppingAction.hh"

SteppingAction::SteppingAction(EventAction *eventAction, bool record_tracks /*=false*/)
{
    E_min = 100. * MeV;
}

SteppingAction::~SteppingAction()
{
}

void SteppingAction::UserSteppingAction(const G4Step *step)
{
    G4Track *track = step->GetTrack();
    G4String pName = track->GetParticleDefinition()->GetParticleName();

    // Kill all neutrinos
    if (pName == "nu_e" || pName == "anti_nu_e" || pName == "nu_mu" || pName == "anti_nu_mu" || pName == "nu_tau" || pName == "anti_nu_tau")
    {
        track->SetTrackStatus(fStopAndKill);
    }

    // Kill particles with energy below E_min
    // or below 0.1 percentage of E_primary
    const G4Event *aevent = G4RunManager::GetRunManager()->GetCurrentEvent();
    G4PrimaryVertex* primaryVertex = aevent->GetPrimaryVertex();
    G4PrimaryParticle* primaryParticle = primaryVertex->GetPrimary();
    G4double E_primary = primaryParticle->GetKineticEnergy();

    G4double E_kinetic = track->GetKineticEnergy();
    if (E_kinetic < E_min || E_kinetic < 1e-3 * E_primary)
    {
        track->SetTrackStatus(fStopAndKill);
    }

    // Record the trajectory of particles (for debugging)
    // G4int eID = G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->GetEventID();
    // G4ThreeVector ppos = step -> GetPreStepPoint() -> GetPosition();
    // G4int Stepid=track->GetCurrentStepNumber();
     
    // record every 1000 steps 
    // if (Stepid % 1000 != 0) return;
    
    // G4String foldername = "output/tracks/";
    // G4String filename = foldername + std::to_string(eID) + pName + ".txt";

    // std::ofstream SDfile;
    // SDfile.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    // if (SDfile.is_open())
    // {
    //     // SDfile<< "  Step:"<<Stepid << "  track: " << aStep->GetTrack()->GetTrackID()
    //     // SDfile<< "  name: " << pTrack ->GetDefinition()->GetParticleName()
    //     SDfile << " " << ppos.getX() / km
    //            << " " << ppos.getY() / km
    //            << " " << ppos.getZ() / km
    //            << G4endl;
    // }

    // print step size (for debugging)
    // G4double step_size = step->GetStepLength();
    // G4cout << "Step size: " << step_size / km << " km" << G4endl;
}
