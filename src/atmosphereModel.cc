#include "atmosphereModel.hh"

LayerAtmosphereModel::LayerAtmosphereModel(DensityModelBase *densityModel)
{
    world_x = 40. * km;
    world_y = 40. * km;

    X_layer = 10. * kg / m2; // Integrated density for every layer, can be adjusted

    h_min = 30. * km;
    h_max = 100. * km;

    sensitive_z = 1. * km;

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
    // ConstructSensitiveLayer();
    return physWorld;
}

void LayerAtmosphereModel::InitBoundaries()
{
    boundaries = {h_min}; // Start with the ground level

    G4double ceilling_max = densityModel->CalculateMaxCeiling(X_layer);

    if (h_max > ceilling_max)
    {
        h_max = ceilling_max;
        G4cout << "Warning: h_max exceeds the maximum ceiling, adjusted to " << h_max / km << " km" << G4endl;
    }

    while (boundaries.back() < h_max)
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
}


void LayerAtmosphereModel::ConstructAtmosphere()
{
    G4double aN = 14.01 * g / mole; // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole; // Molar mass of oxygen

    G4Element *N = new G4Element("Nitrogen", "N", 7, aN);
    G4Element *O = new G4Element("Oxygen", "O", 8, aO);

    G4double epsilon = 1e-6 * m; // Small value 
    G4int nLayers = boundaries.size() - 1; // Number of layers based on the boundaries

    for (int i = 0; i < nLayers; ++i)
    {
        G4double altitude = boundaries[i];
        G4double density = densityModel->CalculateDensity(altitude);

        // G4cout << "Layer " << i << ": Altitude = " << altitude / km << " km, Density = " << density /kg*m3<< " kg/m^3" << G4endl;
        Air.push_back(new G4Material("Air" + std::to_string(i), density, 2));
        Air[i]->AddElement(N, 78 * perCent); // Nitrogen 78%
        Air[i]->AddElement(O, 22 * perCent); // Oxygen 21%

        solidAtmosphere.push_back(new G4Box("solidAtmosphere", world_x, world_y, (boundaries[i + 1] - boundaries[i]) / 2.0 - epsilon));
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
    // solidSensLayer = new G4Box("solidSensLayer", world_x, world_y, sensitive_z);
    // logicSensLayer = new G4LogicalVolume(solidSensLayer,
    //                                      Air[0], // Use the first layer material for the sensitive layer
    //                                      "logicSensLayer");
    // physSensLayer = new G4PVPlacement(0,
    //                                   G4ThreeVector(0., 0., -world_z / nLayers + sensitive_z),
    //                                   logicSensLayer,
    //                                   "physSensLayer",
    //                                   logicAtmosphere[0],
    //                                   false,
    //                                   0,
    //                                   true);
}


G4double LayerAtmosphereModel::GetWorldZ() const
{
    return world_z;
}


void LayerAtmosphereModel::ConstructSDandField()
{
    // Construct sensitive detectors
    // SensitiveDetector *sd = new SensitiveDetector("SensitiveDetector");
    // logicSensLayer->SetSensitiveDetector(sd);
    // G4SDManager *SDManager = G4SDManager::GetSDMpointer();
}