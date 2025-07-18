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
                                           G4double X_layer,
                                           G4double h_min,
                                           G4double h_max,
                                           G4double h_sens)
{
    // User-defined variables
    this->world_x = world_x;
    this->world_y = world_y;
    this->X_layer = X_layer; // Integrated density for every layer
    this->h_min = h_min;
    this->h_max = h_max;
    this->h_sens = h_sens; // location of the sensitive layer

    delta_z = 5. * km;
    delta_sens = 1. * m;

    this->densityModel = densityModel;
    InitBoundaries();
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

void LayerAtmosphereModel::InitBoundaries()
{
    boundaries = {h_min}; // Start with the ground level

    G4double ceiling_max = densityModel->CalculateMaxCeiling(X_layer);

    // filling the space up to maximum ceiling height with fixed integrated density
    while (boundaries.back() < ceiling_max)
    {
        G4double nextBoundary = densityModel->CalculateCeiling(boundaries.back(), X_layer);
        if (std::isnan(nextBoundary))
        {
            G4cout << "Error: nextBoundary is NaN at altitude " << boundaries.back() / km << " km" << G4endl;
            break; // Stop if we encounter a NaN value
        }
        if (nextBoundary > h_max)
            break; // Stop if we exceed the maximum altitude
        boundaries.push_back(nextBoundary);
    }

    // filling the space beyond maximum ceiling height
    if (h_max > ceiling_max)
    {
        G4cout << "h_max exceeds the maximum ceiling height, filling the space "
                  "beyond maximum ceiling height with layer of thickness "
               << delta_z / km << "km" << G4endl;

        while (boundaries.back() < h_max)
        {
            G4double nextBoundary = boundaries.back() + delta_z;
            if (nextBoundary > h_max)
                break; // Stop if we exceed the maximum altitude
            boundaries.push_back(nextBoundary);
        }
    }
}

void LayerAtmosphereModel::ConstructAtmosphere()
{
    G4double aN = 14.01 * g / mole; // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole; // Molar mass of oxygen

    G4Element *N = new G4Element("Nitrogen", "N", 7, aN);
    G4Element *O = new G4Element("Oxygen", "O", 8, aO);

    G4double epsilon = 1e-6 * m;           // Small value to prevent volume overlap
    G4int nLayers = boundaries.size() - 1; // Number of layers based on the boundaries

    for (int i = 0; i < nLayers; ++i)
    {
        G4double altitude = boundaries[i];
        G4double density = densityModel->CalculateDensity(altitude);

        // G4cout << "Layer " << i << ": Altitude = " << altitude / km << " km, Density = " << density /kg*m3<< " kg/m^3" << G4endl;
        Air.push_back(new G4Material("Air" + std::to_string(i), density, 2));
        Air[i]->AddElement(N, 78 * perCent); // Nitrogen 78%
        Air[i]->AddElement(O, 22 * perCent); // Oxygen 21%

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
                               G4double h_min,
                               G4double h_max,
                               G4double h_sens,
                               G4double X_layer)
{
    R_earth = 6371.0 * km; // Global average number

    R_min = h_min + R_earth;
    R_max = h_max + R_earth;
    R_sens = h_sens + R_earth;

    delta_sens = 1. * cm;
    R_delta = 5. * km;

    this->X_layer = X_layer;
    this->densityModel = densityModel;

    InitBoundaries();
}

SphericalEarth::~SphericalEarth()
{
    // Destructor
}

void SphericalEarth::InitBoundaries()
{
    boundaries = {R_min}; // Start with the minimum radius
    G4double ceiling_max = densityModel->CalculateMaxCeiling(X_layer) + R_earth;

    while (boundaries.back() < ceiling_max)
    {
        G4double nextBoundary = densityModel->CalculateCeiling(boundaries.back() - R_earth, X_layer) + R_earth;
        if (std::isnan(nextBoundary))
        {
            G4cout << "Error: nextBoundary is NaN at radius " << boundaries.back() / km << " km" << G4endl;
            break; // Stop if we encounter a NaN value
        }
        if (nextBoundary > R_max)
            break; // Stop if we exceed the maximum radius
        boundaries.push_back(nextBoundary);
    }

    if (R_max > ceiling_max)
    {
        G4cout << "h_max exceeds the maximum ceiling, filling the space "
                  "beyond maximum ceiling height with layer of thickness "
               << R_delta / km << "km" << G4endl;

        while (boundaries.back() < R_max)
        {
            G4double nextBoundary = boundaries.back() + R_delta;
            if (nextBoundary > R_max)
                break; // Stop if we exceed the maximum altitude
            boundaries.push_back(nextBoundary);
        }
    }
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
    G4double aN = 14.01 * g / mole; // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole; // Molar mass of oxygen

    G4Element *N = new G4Element("Nitrogen", "N", 7, aN);
    G4Element *O = new G4Element("Oxygen", "O", 8, aO);

    G4double epsilon = 1e-6 * m;           // Small value to prevent volume overlap
    G4int nLayers = boundaries.size() - 1; // Number of layers based on the boundaries

    for (int i = 0; i < nLayers; ++i)
    {
        G4double altitude = boundaries[i] - R_earth;
        G4double density = densityModel->CalculateDensity(altitude);

        // G4cout << "Layer " << i << ": Altitude = " << altitude / km << " km, Density = " << density /kg*m3<< " kg/m^3" << G4endl;
        Air.push_back(new G4Material("Air" + std::to_string(i), density, 2));
        Air[i]->AddElement(N, 78 * perCent); // Nitrogen 78%
        Air[i]->AddElement(O, 22 * perCent); // Oxygen 21%

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