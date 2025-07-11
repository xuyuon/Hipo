#include "runAction.hh"

RunAction::RunAction()
{
    // Constructor implementation
}

RunAction::~RunAction()
{
    // Destructor implementation
}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

    G4int runID = run->GetRunID();
    std::stringstream strRunID;
    strRunID << runID;

    // G4bool fileOpen = analysisManager->OpenFile("output.hdf5"); 
    G4bool fileOpen = analysisManager->OpenFile("output"+strRunID.str()+".csv");

    if (!fileOpen) {
        G4cout << "Error: Could not open output file!" << G4endl;
        return;
    }

    analysisManager->CreateNtuple("Gamma", "Gamma");
    analysisManager->CreateNtupleDColumn("energy");
    analysisManager->CreateNtupleSColumn("process");
    analysisManager->CreateNtupleDColumn("position_r");
    analysisManager->CreateNtupleDColumn("position_phi");
    analysisManager->CreateNtupleDColumn("position_theta");
    analysisManager->CreateNtupleDColumn("nadir_phi");
    analysisManager->CreateNtupleDColumn("nadir_theta");
    analysisManager->FinishNtuple(0);

    analysisManager->CreateNtuple("Incident_p", "Incident_p");
    analysisManager->CreateNtupleDColumn("energy");
    analysisManager->CreateNtupleDColumn("position_r");
    analysisManager->CreateNtupleDColumn("position_phi");
    analysisManager->CreateNtupleDColumn("position_theta");
    analysisManager->CreateNtupleDColumn("nadir_phi");
    analysisManager->CreateNtupleDColumn("nadir_theta");
    analysisManager->FinishNtuple(1);
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}