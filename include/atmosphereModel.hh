#ifndef ATMOSPHEREMODEL_HH
#define ATMOSPHEREMODEL_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh" 
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"


const int nLayers = 20; // Number of atmospheric layers



class AtmosphereModelBase : public G4VUserDetectorConstruction
{
public:
    AtmosphereModelBase(){}
    virtual ~AtmosphereModelBase(){}

    virtual G4VPhysicalVolume *Construct() = 0;
};


class LayerAtmosphereModel : public AtmosphereModelBase
{
public:
    LayerAtmosphereModel();
    ~LayerAtmosphereModel();

    virtual G4VPhysicalVolume *Construct() override;

    double CalculateDensity(G4double altitude);

private:
    G4double world_x, world_y, world_z; // half lengths of the world volume
    
    // Volumes
    G4Box *solidWorld, *solidAtmosphere; 
    G4LogicalVolume *logicWorld, *logicAtmosphere[nLayers];
    G4VPhysicalVolume *physWorld, *physAtmosphere[nLayers];

    

    void ConstructAtmosphere();
};


#endif 
