#include "run.hh"

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
    G4AnalysisManager *man = G4AnalysisManager::Instance();

    G4bool fileOpen = man->OpenFile("output.root"); 

    if (!fileOpen) {
        G4cout << "Error: Could not open output file!" << G4endl;
        return;
    }

    man->CreateNtuple("Hits", "Hits");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->FinishNtuple(0);
}

void MyRunAction::EndOfRunAction(const G4Run*)
{
    G4AnalysisManager *man = G4AnalysisManager::Instance();

    man->Write();
    man->CloseFile();
}