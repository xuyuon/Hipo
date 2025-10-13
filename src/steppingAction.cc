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

    if (pName == "nu_e" || pName == "anti_nu_e" || pName == "nu_mu" || pName == "anti_nu_mu" || pName == "nu_tau" || pName == "anti_nu_tau")
    {
        track->SetTrackStatus(fStopAndKill);
    }

    // Kill particles with energy below E_min
    G4double E_kinetic = track->GetKineticEnergy();
    if (E_kinetic < E_min && pName == "gamma")
    {
        track->SetTrackStatus(fStopAndKill);
    }

    // Record the trajectory of particles
    G4int eID = G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->GetEventID();
    G4ThreeVector ppos = step -> GetPreStepPoint() -> GetPosition();

    G4String foldername = "output/tracks/";
    G4String filename = foldername + std::to_string(eID) + pName + ".txt";

    std::ofstream SDfile;
    SDfile.open(filename, std::ios::app);
    if (SDfile.is_open())
    {
        // SDfile<< "  Step:"<<Stepid << "  track: " << aStep->GetTrack()->GetTrackID()
        // SDfile<< "  name: " << pTrack ->GetDefinition()->GetParticleName()
        SDfile << " " << ppos.getX() / km
               << " " << ppos.getY() / km
               << " " << ppos.getZ() / km
               << G4endl;
    }

    // print step size (for debugging)
    // G4double step_size = step->GetStepLength();
    // G4cout << "Step size: " << step_size / km << " km" << G4endl;
}
