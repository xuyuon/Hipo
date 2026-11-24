#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4VProcess.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"

#include "outputManager.hh"

#include <iostream>
#include <fstream>
#include <iomanip>

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
                     OutputManager *outputManager = nullptr,
                     int sens_layer_id = 0);
    ~SphericalShellSD();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;
    static void p_mom_to_nadir_angles(const G4ThreeVector *p_pos,
                               const G4ThreeVector *p_mom,
                               G4double *nadir_angles);

private:
    OutputManager *outputManager;
    int sens_layer_id;
    G4String particle;
};

#endif
