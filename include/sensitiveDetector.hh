#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4VProcess.hh"


class SensitiveDetector : public G4VSensitiveDetector
{
public:
    SensitiveDetector(G4String name);
    ~SensitiveDetector();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);

private:
    G4AnalysisManager *analysisManager; // singleton instance of analysis manager
};


#endif