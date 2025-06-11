#include "atmosphereModel.hh"

LayerAtmosphereModel::LayerAtmosphereModel()
{
    world_x = 40. * km;
    world_y = 40. * km;
    world_z = 100. * km;

    sensitive_z = 1. * km;
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
    G4double aN = 14.01 * g / mole; // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole; // Molar mass of oxygen

    G4Element *N = new G4Element("Nitrogen", "N", 7, aN);
    G4Element *O = new G4Element("Oxygen", "O", 8, aO);

    solidAtmosphere = new G4Box("solidAtmosphere", world_x, world_y, world_z / nLayers);

    for (int i = 0; i < nLayers; ++i)
    {
        G4double altitude = 2 * world_z / nLayers * i;
        G4double density = CalculateDensity(altitude);

        // G4cout << "Layer " << i << ": Altitude = " << altitude / km << " km, Density = " << density /kg*m3<< " kg/m^3" << G4endl;
        Air[i] = new G4Material("Air" + std::to_string(i), density, 2);
        Air[i]->AddElement(N, 78 * perCent); // Nitrogen 78%
        Air[i]->AddElement(O, 22 * perCent); // Oxygen 21%

        logicAtmosphere[i] = new G4LogicalVolume(solidAtmosphere, Air[i], "logicAtmosphere");
        physAtmosphere[i] = new G4PVPlacement(0,
                                              G4ThreeVector(0., 0., -world_z + world_z / nLayers * (1.0 + double(i) * 2)),
                                              logicAtmosphere[i],
                                              "physAtmosphere",
                                              logicWorld,
                                              false,
                                              i,
                                              true);
    }
}

void LayerAtmosphereModel::ConstructSensitiveLayer()
{
    solidSensLayer = new G4Box("solidSensLayer", world_x, world_y, sensitive_z);
    logicSensLayer = new G4LogicalVolume(solidSensLayer,
                                         Air[0], // Use the first layer material for the sensitive layer
                                         "logicSensLayer");
    physSensLayer = new G4PVPlacement(0,
                                      G4ThreeVector(0., 0., -world_z/nLayers + sensitive_z),
                                      logicSensLayer,
                                      "physSensLayer",
                                      logicAtmosphere[0],
                                      false,
                                      0,
                                      true);
}

G4double LayerAtmosphereModel::CalculateDensity(G4double altitude)
{
    /*
    Calculates the density of air (in newtons per square meter) at a given altitude (in m)
    using the barometric formula.
    */

    G4double R = 8.3144626181532; // ideal gas constant
    G4double g = 9.81;            // gravitational acceleration

    G4double m_air = 0.028965; // molar mass of dry air

    G4double h_ref = 50000.0;  // We set reference height at 50km (where most of photons are produced)
    G4double temp_ref = 260.0; // Reference temperature at 50km
    // source: https://amt.copernicus.org/articles/15/2231/2022/amt-15-2231-2022-f10-high-res.pdf
    G4double pressure_ref = 79.77; // Reference pressure at 50km
    // source: https://www.pdas.com/atmosTable1SI.html

    G4double density = m_air / R / temp_ref * pressure_ref * exp(-g * m_air * (altitude / m - h_ref) / (R * temp_ref)) * kg / m3;
    return density;
}

void LayerAtmosphereModel::ConstructSDandField()
{
    // Construct sensitive detectors
    SensitiveDetector *sd = new SensitiveDetector("SensitiveDetector");
    logicSensLayer->SetSensitiveDetector(sd);
    // G4SDManager *SDManager = G4SDManager::GetSDMpointer();
}