#ifndef ATMOSPHEREMODEL_HH
#define ATMOSPHEREMODEL_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"

#include <vector>
#include <cmath>

#include "sensitiveDetector.hh"
#include "densityModel.hh"


class AtmosphereModelBase : public G4VUserDetectorConstruction
{
public:
    AtmosphereModelBase() {}
    virtual ~AtmosphereModelBase() {}

    virtual G4VPhysicalVolume *Construct() = 0;
    virtual G4double GetWorldZ() const = 0;
};

class LayerAtmosphereModel : public AtmosphereModelBase
{
public:
    LayerAtmosphereModel(DensityModelBase*, 
                         G4double world_x,
                         G4double world_y,
                         G4double X_layer, // Integrated density for every layer
                         G4double h_min,
                         G4double h_max,
                         G4double sens_z);
    ~LayerAtmosphereModel();

    virtual G4VPhysicalVolume *Construct() override;
    virtual G4double GetWorldZ() const override;
    void ConstructSDandField() override;
private:
    G4double world_x, world_y, world_z; // half lengths of the world volume
    G4double h_min, h_max; // minimum and maximum altitudes in the atmosphere
    G4double X_layer; // Integrated density for every layer
    G4double sens_z;               // half-width of the sensitive layer
    G4int sens_layer;           // sensitive layer index

    // Volumes
    G4Box *solidWorld;
    std::vector<G4Box *> solidAtmosphere;
    G4LogicalVolume *logicWorld;
    std::vector<G4LogicalVolume *> logicAtmosphere;
    G4VPhysicalVolume *physWorld;
    std::vector<G4VPhysicalVolume *> physAtmosphere;

    // Materials
    G4Material *worldMat;
    std::vector<G4Material *> Air;
    
    // Density profile model
    DensityModelBase *densityModel;
    std::vector<G4double> boundaries;

    void InitBoundaries();
    void ConstructAtmosphere();
};

#endif
