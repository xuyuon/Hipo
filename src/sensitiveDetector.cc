# include "sensitiveDetector.hh"


SensitiveDetector::SensitiveDetector(G4String name) : G4VSensitiveDetector(name)
{

}

SensitiveDetector::~SensitiveDetector()
{

}

G4bool SensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* ROhist)
{
    G4Track* track = aStep->GetTrack();
    G4String pName = track->GetDefinition()->GetParticleName();

    // we are only interested in recording photons
    if (pName != "gamma") return false;

    G4StepPoint* preStepPoint = aStep->GetPreStepPoint(); // when the particle enters the sensitive detector
    G4StepPoint* postStepPoint = aStep->GetPostStepPoint(); // when the particle exits the sensitive detector

    G4double energy = preStepPoint->GetKineticEnergy(); 

    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager
    analysisManager->FillNtupleDColumn(0, 0, energy/eV); 
    analysisManager->AddNtupleRow(0); 


    return true;
}