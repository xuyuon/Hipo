#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH


#include "G4VUserDetectorConstruction.hh" 
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh" 
#include "G4PVPlacement.hh" 
#include "G4Box.hh" 
#include "G4SystemOfUnits.hh" // Include Geant4 units


class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume *Construct();
    void ConstructAtmosphere();

private:
    G4double world_hx, world_hy, world_hz; // half lengths of the world volume

    G4Box *solidWorld, *solidAtmosphere;
    G4LogicalVolume *logicWorld, *logicAtmosphere[10]; 
    G4VPhysicalVolume *physWorld, *physAtmosphere[10];

    G4Material *worldMat, *Air[10];
    G4Element *N, *O;

    G4LogicalVolume *logicDetector; 

    void DefineMaterials();
    
};

#endif