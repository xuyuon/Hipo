#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4VProcess.hh"
#include "G4EventManager.hh"

#include <iostream>
#include <fstream>
#include <filesystem>

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
    SphericalShellSD(G4String name,
                     G4String particle = "gamma",
                     std::string SDOutputDir = "output/sensitiveDetector");
    ~SphericalShellSD();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;

private:
    std::string SDOutputDir, SDOutputFile;
    G4String particle;
};

#endif