#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4SystemOfUnits.hh"


class SensitiveDetector : public G4VSensitiveDetector
{
public:
    SensitiveDetector(G4String name);
    ~SensitiveDetector();

    G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);
};


#endif