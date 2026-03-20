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
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4DormandPrince745.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4ChordFinder.hh"
#include "G4UserLimits.hh"
#include "G4PropagatorInField.hh"

#include <vector>
#include <cmath>

#include "sensitiveDetector.hh"
#include "densityModel.hh"
#include "field.hh"
#include "fileManager.hh"

class AtmosphereModelBase : public G4VUserDetectorConstruction
{
public:
    AtmosphereModelBase() {}
    virtual ~AtmosphereModelBase() {}

    virtual G4VPhysicalVolume *Construct() = 0;
};

class SphericalEarth : public AtmosphereModelBase
{
    /*
    A spherical Earth model with a given radius and a layer of air above it.
    */
public:
    SphericalEarth(DensityModelBase *densityModel,
                   std::vector<G4double> R_sens, // Height of the sensitive layer
                   G4double R_earth,
                   G4MagneticField *Bfield = nullptr,
                   std::string particle_name = "gamma",
                   FileManager *fileManager = nullptr
    );
    ~SphericalEarth();

    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;

private:
    // Geometry
    G4double R_earth; // Radius of the Earth
    std::vector<G4double> R_sens; // List of position of sensitive layers
    G4double delta_sens; // thickness of sensitive layer
    std::vector<int> sens_layer;    // Indices of the sensitive layers
    int nSensLayers; // Number of sensitive detector layers

    // Density model
    DensityModelBase *densityModel;
    G4int nLayers_r;     // Number of altitude layers
    G4int nLayers_theta; // Number of latitude layers
    G4int nLayers_phi;   // Number of longitude layers

    // Volumes
    G4Sphere *solidWorld;
    G4LogicalVolume *logicWorld;
    G4VPhysicalVolume *physWorld;
    std::vector<std::vector<std::vector<G4Sphere *>>> solidAtmosphere;
    std::vector<std::vector<std::vector<G4LogicalVolume *>>> logicAtmosphere;
    std::vector<std::vector<std::vector<G4VPhysicalVolume *>>> physAtmosphere;
    std::vector<std::vector<std::vector<G4Sphere *>>> solidSensLayer; // [n_layer][nLayers_theta][nLayers_phi]
    std::vector<std::vector<std::vector<G4LogicalVolume *>>> logicSensLayer;
    std::vector<std::vector<std::vector<G4VPhysicalVolume *>>> physSensLayer;

    // Materials
    G4Material *worldMat;
    std::vector<std::vector<std::vector<G4Material *>>> Air; // [altitude][glat][glon]

    // Fields
    G4MagneticField *Bfield;
    
    // Particle to be detected
    std::string particle_name;

    // File manager
    FileManager *fileManager;

    // User Limits
    G4UserLimits *userLimits;

    // Private Methods
    void ConstructAtmosphere();
    void ConstructSensitiveLayer();
};

class Magnetosphere : public AtmosphereModelBase
{
    /*
    The outer spherical shell of Earth
    Only magnetic field is considered here
    */
public:
    Magnetosphere(G4double R_inner,
                  G4double R_outer,
                  std::vector<G4double> R_sens,
                  G4MagneticField *Bfield = nullptr,
                  std::string particle_name = "anti_proton",
                  FileManager *fileManager = nullptr);
    ~Magnetosphere();

    virtual G4VPhysicalVolume *Construct() override;
    void ConstructSDandField() override;

private:
    // User Limits
    G4UserLimits *userLimits;

    // Geometry
    G4double R_inner, R_outer; // Inner and outer radius of the magnetosphere
    std::vector<G4double> R_sens;

    // Volumes
    G4Sphere *solidWorld, *solidSensLayer;
    G4LogicalVolume *logicWorld, *logicSensLayer;
    G4VPhysicalVolume *physWorld, *physSensLayer;

    // Materials
    G4Material *worldMat;

    // Fields
    G4MagneticField *Bfield;

    // Particle to be detected
    std::string particle_name;

    // Output directory
    FileManager *fileManager;

    // Private Methods
    void ConstructSensitiveLayer();
};

#endif
