#ifndef ATMOSPHEREMODEL_HH
#define ATMOSPHEREMODEL_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"

#include "sensitiveDetector.hh"
#include "densityModel.hh"


const int nLayers = 20; // Number of atmospheric layers

class AtmosphereModelBase : public G4VUserDetectorConstruction
{
public:
    AtmosphereModelBase() {}
    virtual ~AtmosphereModelBase() {}

    virtual G4VPhysicalVolume *Construct() = 0;
};

class LayerAtmosphereModel : public AtmosphereModelBase
{
public:
    LayerAtmosphereModel(DensityModelBase*);
    ~LayerAtmosphereModel();


    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;
private:
    G4double world_x, world_y, world_z; // half lengths of the world volume
    G4double sensitive_z;               // half-width of the sensitive layer

    // Volumes
    G4Box *solidWorld, *solidAtmosphere, *solidSensLayer;
    G4LogicalVolume *logicWorld, *logicAtmosphere[nLayers],
        *logicSensLayer;
    G4VPhysicalVolume *physWorld, *physAtmosphere[nLayers],
        *physSensLayer;

    // Materials
    G4Material *worldMat, *Air[nLayers];
    // Density profile model
    DensityModelBase *densityModel;

    void ConstructAtmosphere();
    void ConstructSensitiveLayer();
};

#endif
