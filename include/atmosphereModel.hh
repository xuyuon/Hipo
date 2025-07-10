#ifndef ATMOSPHEREMODEL_HH
#define ATMOSPHEREMODEL_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
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
};

class AirBlock : public AtmosphereModelBase
{
    /*
    A simple block of air for testing purpose
    */
public:
    AirBlock(G4double world_x, G4double world_y, G4double world_z);
    ~AirBlock();

    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;

private:
    G4double world_x, world_y, world_z; // half lengths of the world volume

    // Volumes
    G4Box *solidWorld;
    G4LogicalVolume *logicWorld;
    G4VPhysicalVolume *physWorld;

    // Materials
    G4Material *worldMat;
};

class LayerAtmosphereModel : public AtmosphereModelBase
{
public:
    LayerAtmosphereModel(DensityModelBase *,
                         G4double world_x,
                         G4double world_y,
                         G4double X_layer, // Integrated density for every layer
                         G4double h_min,
                         G4double h_max,
                         G4double h_sens);
    ~LayerAtmosphereModel();

    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;
    G4double GetCeiling() const; // return maximum height

private:
    // Geometric Parameters
    G4double world_x, world_y, world_z; // half lengths of the world volume
    G4double delta_z;                   // thickness of each layer
    G4double h_min, h_max;              // minimum and maximum altitudes in the atmosphere
    G4double X_layer;                   // Integrated density for every layer
    G4double h_sens;                    // altitude of sensitive layer
    G4double delta_sens;                // thichkness of the sensitive layer
    G4int sens_layer;                   // sensitive layer index

    // Volumes
    G4Box *solidWorld, *solidSensLayer;
    std::vector<G4Box *> solidAtmosphere;
    G4LogicalVolume *logicWorld, *logicSensLayer;
    std::vector<G4LogicalVolume *> logicAtmosphere;
    G4VPhysicalVolume *physWorld, *physSensLayer;
    std::vector<G4VPhysicalVolume *> physAtmosphere;

    // Materials
    G4Material *worldMat;
    std::vector<G4Material *> Air;

    // Density profile model
    DensityModelBase *densityModel;
    std::vector<G4double> boundaries;

    void InitBoundaries();
    void ConstructAtmosphere();
    void ConstructSensitiveLayer();
};

class SphericalEarth : public AtmosphereModelBase
{
    /*
    A spherical Earth model with a given radius and a layer of air above it.
    */
public:
    SphericalEarth(DensityModelBase *densityModel,
                   G4double h_min,   // Minimum height of the air layer
                   G4double h_max,   // Maximum height of the air layer
                   G4double h_sens,  // Height of the sensitive layer
                   G4double X_layer  // Integrated density for the air layer
    );
    ~SphericalEarth();

    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;
    G4double GetCeiling() const; // return maximum height

private:
    // Geometry
    G4double R_earth; // Radius of the Earth
    G4double R_min, R_max;
    G4double R_delta; // thickness of each layer
    G4double R_sens;
    G4double delta_sens; // thickness of sensitive layer
    G4int sens_layer; // Index of the sensitive layer

    // Integrated density for the air layer
    G4double X_layer;

    // Density model
    DensityModelBase *densityModel;
    std::vector<G4double> boundaries;

    // Volumes
    G4Sphere *solidWorld, *solidSensLayer;
    std::vector<G4Sphere *> solidAtmosphere;
    G4LogicalVolume *logicWorld, *logicSensLayer;
    std::vector<G4LogicalVolume *> logicAtmosphere;
    G4VPhysicalVolume *physWorld, *physSensLayer;
    std::vector<G4VPhysicalVolume *> physAtmosphere;

    // Materials
    G4Material *worldMat;
    std::vector<G4Material *> Air;

    // Private Methods
    void InitBoundaries();
    void ConstructAtmosphere();
    void ConstructSensitiveLayer();
};

#endif
