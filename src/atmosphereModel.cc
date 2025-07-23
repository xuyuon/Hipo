#include "atmosphereModel.hh"

AirBlock::AirBlock(G4double world_x, G4double world_y, G4double world_z)
{
    // User-defined variables
    this->world_x = world_x;
    this->world_y = world_y;
    this->world_z = world_z;
}

AirBlock::~AirBlock()
{
    // Destructor
}

G4VPhysicalVolume *AirBlock::Construct()
{
    // Define world material
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // Create the world volume
    solidWorld = new G4Box("solidWorld", world_x, world_y, world_z);
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    physWorld = new G4PVPlacement(0,
                                  G4ThreeVector(0., 0., 0.),
                                  logicWorld,
                                  "physWorld",
                                  0,
                                  false,
                                  0,
                                  true);
    return physWorld;
}

void AirBlock::ConstructSDandField()
{
}

LayerAtmosphereModel::LayerAtmosphereModel(DensityModelBase *densityModel,
                                           G4double world_x,
                                           G4double world_y,
                                           G4double h_sens)
{
    // User-defined variables
    this->world_x = world_x;
    this->world_y = world_y;
    this->h_sens = h_sens; // location of the sensitive layer

    delta_h = 5. * km;
    delta_sens = 1. * m;

    this->densityModel = densityModel;
    this->boundaries = densityModel->GetBoundaries();
    this->densities = densityModel->GetDensities();

    world_z = (boundaries.back() - boundaries.front()) / 2.0; // Adjust world_z to the maximum altitude defined by the density model
}

LayerAtmosphereModel::~LayerAtmosphereModel()
{
    // Destructor
}

G4VPhysicalVolume *LayerAtmosphereModel::Construct()
{
    // Define world material
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // Create the world volume
    solidWorld = new G4Box("solidWorld", world_x, world_y, world_z);
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    physWorld = new G4PVPlacement(0,
                                  G4ThreeVector(0., 0., 0.),
                                  logicWorld,
                                  "physWorld",
                                  0,
                                  false,
                                  0,
                                  true);

    ConstructAtmosphere();
    ConstructSensitiveLayer();
    return physWorld;
}

void LayerAtmosphereModel::ConstructAtmosphere()
{
    G4double aN = 14.01 * g / mole;  // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole;  // Molar mass of oxygen
    G4double aAr = 39.95 * g / mole; // Molar mass of argon
    G4double aHe = 4.003 * g / mole; // Molar mass of helium
    G4double aH = 1.008 * g / mole;  // Molar mass of hydrogen

    G4Element *N = new G4Element("Nitrogen", "N", 7., aN);
    G4Element *O = new G4Element("Oxygen", "O", 8., aO);
    G4Element *Ar = new G4Element("Argon", "Ar", 18., aAr);
    G4Element *He = new G4Element("Helium", "He", 2., aHe);
    G4Element *H = new G4Element("Hydrogen", "H", 1., aH);

    G4double epsilon = 1e-6 * m;           // Small value to prevent volume overlap
    G4int nLayers = boundaries.size() - 1; // Number of layers based on the boundaries

    for (int i = 0; i < nLayers; ++i)
    {
        G4double altitude = boundaries[i];
        G4double density = densities[i];

        Air.push_back(new G4Material("Air" + std::to_string(i), density, 5));
        Air[i]->AddElement(N, densityModel->GetNPercentageMass()[i]); 
        Air[i]->AddElement(O, densityModel->GetOPercentageMass()[i]);
        Air[i]->AddElement(Ar, densityModel->GetArPercentageMass()[i]);
        Air[i]->AddElement(He, densityModel->GetHePercentageMass()[i]);
        Air[i]->AddElement(H, densityModel->GetHPercentageMass()[i]); 

        if (h_sens > boundaries[i] && h_sens < boundaries[i + 1])
        {
            sens_layer = i; // Set the sensitive layer index
        }

        solidAtmosphere.push_back(new G4Box("solidAtmosphere",
                                            world_x,
                                            world_y,
                                            (boundaries[i + 1] - boundaries[i]) / 2.0 - epsilon));
        logicAtmosphere.push_back(new G4LogicalVolume(solidAtmosphere[i], Air[i], "logicAtmosphere"));
        physAtmosphere.push_back(new G4PVPlacement(0,
                                                   G4ThreeVector(0., 0., -world_z + (boundaries[i] - boundaries[0]) + (boundaries[i + 1] - boundaries[i]) / 2.0),
                                                   logicAtmosphere[i],
                                                   "physAtmosphere",
                                                   logicWorld,
                                                   false,
                                                   i,
                                                   true));
    }
}

void LayerAtmosphereModel::ConstructSensitiveLayer()
{
    solidSensLayer = new G4Box("solidSensLayer", world_x, world_y, delta_sens);
    logicSensLayer = new G4LogicalVolume(solidSensLayer,
                                         Air[sens_layer], // Use the first layer material for the sensitive layer
                                         "logicSensLayer");
    physSensLayer = new G4PVPlacement(0,
                                      G4ThreeVector(0., 0., -(boundaries[sens_layer + 1] + boundaries[sens_layer] + delta_sens) / 2.0 + h_sens),
                                      logicSensLayer,
                                      "physSensLayer",
                                      logicAtmosphere[sens_layer],
                                      false,
                                      0,
                                      true);
}

G4double LayerAtmosphereModel::GetCeiling() const
{
    return boundaries.back();
}

void LayerAtmosphereModel::ConstructSDandField()
{
    // Construct sensitive detectors
    SensitiveDetectorBase *sd = new SensitiveDetectorBase("SensitiveDetector");
    logicSensLayer->SetSensitiveDetector(sd);
    // G4SDManager *SDManager = G4SDManager::GetSDMpointer();
}

SphericalEarth::SphericalEarth(DensityModelBase *densityModel,
                               G4double h_sens)
{
    R_earth = 6371.0 * km; // Global average number

    R_sens = h_sens + R_earth;
    delta_sens = 1. * cm;

    this->densityModel = densityModel;
    this->boundaries = densityModel->GetBoundaries();
    this->densities = densityModel->GetDensities();

    // Convert boundaries from altitude to radius
    for (auto &boundary : boundaries)
    {
        boundary += R_earth;
    }
}

SphericalEarth::~SphericalEarth()
{
    // Destructor
}

G4VPhysicalVolume *SphericalEarth::Construct()
{
    // Define world material
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // Create the world volume
    solidWorld = new G4Sphere("solidWorld",
                              R_earth,
                              boundaries.back(),
                              0 * degree,
                              360 * degree,
                              0. * degree,
                              180. * degree);
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    physWorld = new G4PVPlacement(0,
                                  G4ThreeVector(0., 0., 0.),
                                  logicWorld,
                                  "physWorld",
                                  0,
                                  false,
                                  0,
                                  true);
    ConstructAtmosphere();
    ConstructSensitiveLayer();
    return physWorld;
}

void SphericalEarth::ConstructAtmosphere()
{
    G4double aN = 14.01 * g / mole;  // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole;  // Molar mass of oxygen
    G4double aAr = 39.95 * g / mole; // Molar mass of argon
    G4double aHe = 4.003 * g / mole; // Molar mass of helium
    G4double aH = 1.008 * g / mole;  // Molar mass of hydrogen

    G4Element *N = new G4Element("Nitrogen", "N", 7., aN);
    G4Element *O = new G4Element("Oxygen", "O", 8., aO);
    G4Element *Ar = new G4Element("Argon", "Ar", 18., aAr);
    G4Element *He = new G4Element("Helium", "He", 2., aHe);
    G4Element *H = new G4Element("Hydrogen", "H", 1., aH);

    G4double epsilon = 1e-6 * m;           // Small value to prevent volume overlap
    G4int nLayers = boundaries.size() - 1; // Number of layers based on the boundaries

    for (int i = 0; i < nLayers; ++i)
    {
        Air.push_back(new G4Material("Air" + std::to_string(i), densities[i], 5));
        Air[i]->AddElement(N, densityModel->GetNPercentageMass()[i]); 
        Air[i]->AddElement(O, densityModel->GetOPercentageMass()[i]);
        Air[i]->AddElement(Ar, densityModel->GetArPercentageMass()[i]);
        Air[i]->AddElement(He, densityModel->GetHePercentageMass()[i]);
        Air[i]->AddElement(H, densityModel->GetHPercentageMass()[i]); 

        if (R_sens > boundaries[i] && R_sens < boundaries[i + 1])
        {
            sens_layer = i; // Set the sensitive layer index
        }

        solidAtmosphere.push_back(new G4Sphere("solidAtmosphere",
                                               boundaries[i],
                                               boundaries[i + 1] - epsilon,
                                               0 * degree,
                                               360 * degree,
                                               0. * degree,
                                               180. * degree));
        logicAtmosphere.push_back(new G4LogicalVolume(solidAtmosphere[i], Air[i], "logicAtmosphere"));
        physAtmosphere.push_back(new G4PVPlacement(0,
                                                   G4ThreeVector(0., 0., 0.),
                                                   logicAtmosphere[i],
                                                   "physAtmosphere",
                                                   logicWorld,
                                                   false,
                                                   i,
                                                   true));
    }
}

void SphericalEarth::ConstructSensitiveLayer()
{
    solidSensLayer = new G4Sphere("solidSensLayer",
                                  R_sens,
                                  R_sens + delta_sens,
                                  0 * degree,
                                  360 * degree,
                                  0. * degree,
                                  180. * degree);
    logicSensLayer = new G4LogicalVolume(solidSensLayer,
                                         Air[sens_layer],
                                         "logicSensLayer");
    physSensLayer = new G4PVPlacement(0,
                                      G4ThreeVector(0., 0., 0.),
                                      logicSensLayer,
                                      "physSensLayer",
                                      logicAtmosphere[sens_layer],
                                      false,
                                      0,
                                      true);
}

G4double SphericalEarth::GetCeiling() const
{
    return boundaries.back();
}

void SphericalEarth::ConstructSDandField()
{
    SensitiveDetectorBase *sd = new SphericalShellSD("SensitiveDetector");
    logicSensLayer->SetSensitiveDetector(sd);
}