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
    analysisManager->FinishNtuple(0);
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
}