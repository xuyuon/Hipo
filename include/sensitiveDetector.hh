#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4VProcess.hh"


class SensitiveDetectorBase : public G4VSensitiveDetector
{
public:
    SensitiveDetectorBase(G4String name);
    ~SensitiveDetectorBase();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);

protected:
    G4AnalysisManager *analysisManager; // singleton instance of analysis manager
};


class SphericalShellSD : public SensitiveDetectorBase
{
public:
    SphericalShellSD(G4String name);
    ~SphericalShellSD();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;

};


#endif