#include "sensitiveDetector.hh"

SensitiveDetectorBase::SensitiveDetectorBase(G4String name) : G4VSensitiveDetector(name)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager
}

SensitiveDetectorBase::~SensitiveDetectorBase()
{
}

G4bool SensitiveDetectorBase::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    G4Track *track = aStep->GetTrack();
    G4String pName = track->GetDefinition()->GetParticleName();

    // we are only interested in recording photons
    if (pName != "gamma")
        return false;

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();   // when the particle enters the sensitive detector
    G4StepPoint *postStepPoint = aStep->GetPostStepPoint(); // when the particle exits the sensitive detector

    // Record the energy of the photon at the pre-step point
    G4double energy = preStepPoint->GetKineticEnergy();
    analysisManager->FillNtupleDColumn(0, 0, energy / eV);

    // Record the physical process that generated the photon
    G4String processName = track->GetCreatorProcess()->GetProcessName();
    analysisManager->FillNtupleSColumn(0, 1, processName);

    analysisManager->AddNtupleRow(0);

    return true;
}


SphericalShellSD::SphericalShellSD(G4String name) : SensitiveDetectorBase(name)
{
}   

SphericalShellSD::~SphericalShellSD()
{
}

G4bool SphericalShellSD::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    G4Track *track = aStep->GetTrack();
    G4String pName = track->GetDefinition()->GetParticleName();

    // we are only interested in recording photons
    if (pName != "gamma")
        return false;

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();   // when the particle enters the sensitive detector
    G4StepPoint *postStepPoint = aStep->GetPostStepPoint(); // when the particle exits the sensitive detector

    // Record the energy of the photon at the pre-step point
    G4double energy = preStepPoint->GetKineticEnergy();
    analysisManager->FillNtupleDColumn(0, 0, energy / eV);

    // Record the physical process that generated the photon
    G4String processName = track->GetCreatorProcess()->GetProcessName();
    analysisManager->FillNtupleSColumn(0, 1, processName);

    // Record the position of the photon 
    G4ThreeVector p_pos = track->GetPosition();
    analysisManager->FillNtupleDColumn(0, 2, p_pos.mag() / km);
    analysisManager->FillNtupleDColumn(0, 3, p_pos.phi() / radian);
    analysisManager->FillNtupleDColumn(0, 4, p_pos.theta() / radian);

    // Record the momentum direction of the photon
    G4ThreeVector p_mom = track->GetMomentumDirection();

    analysisManager->AddNtupleRow(0);

    return true;
}

