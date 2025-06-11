#include "runAction.hh"

MyRunAction::MyRunAction()
{
    // Constructor implementation
}

MyRunAction::~MyRunAction()
{
    // Destructor implementation
}

void MyRunAction::BeginOfRunAction(const G4Run*)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

    G4bool fileOpen = analysisManager->OpenFile("output.hdf5"); 

    if (!fileOpen) {
        G4cout << "Error: Could not open output file!" << G4endl;
        return;
    }

    analysisManager->CreateNtuple("Hits", "Hits");
    analysisManager->CreateNtupleIColumn("fEvent");
    analysisManager->CreateNtupleDColumn("fX");
    analysisManager->CreateNtupleDColumn("fY");
    analysisManager->CreateNtupleDColumn("fZ");
    analysisManager->FinishNtuple(0);
}

void MyRunAction::EndOfRunAction(const G4Run*)
{
    G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

    analysisManager->Write();
    analysisManager->CloseFile();
}