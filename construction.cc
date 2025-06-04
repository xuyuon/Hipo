#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
{
    // Constructor implementation
    DefineMaterials();
    world_hx = 40. *km;
    world_hy = 40. *km;
    world_hz = 20. *km;
}

MyDetectorConstruction::~MyDetectorConstruction()
{
    // Destructor implementation
}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    solidWorld = new G4Box("solidWorld", world_hx, world_hy, world_hz);
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

    return physWorld;
}

void MyDetectorConstruction::DefineMaterials()
{
    // Define materials
    G4NistManager *nist = G4NistManager::Instance();


    worldMat = nist->FindOrBuildMaterial("G4_AIR");

    G4double energy[2] = {1.239841939*eV/0.9, 1.239841939*eV/0.2};
    G4double rindexWorld[2] = {1.0, 1.0};

    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energy, rindexWorld, 2);

    worldMat->SetMaterialPropertiesTable(mptWorld);


    G4double density0 = 1.29 * kg / m3; // Density of air at sea level
    G4double aN = 14.01 * g / mole;     // Molar mass of nitrogen
    G4double aO = 16.00 * g / mole;     // Molar mass of oxygen

    N = new G4Element("Nitrogen", "N", 7, aN);
    O = new G4Element("Oxygen", "O", 8, aO);

    G4double f = 3;               // degree of freedom
    G4double R = 8.3144626181532; // ideal gas constant
    G4double g0 = 9.81;           // gravitational acceleration
    G4double kappa = (f + 2) / f;
    G4double T = 293.15;                        // room temperature in Kelvin
    G4double M = (0.3 * aO + 0.7 * aN) / 1000.; // average molar mass

    for (G4int i = 0; i < 10; i++)
    {
        std::stringstream stri;
        stri << i;
        G4double h = 40e3 / 10. * i; // height of each layer in meters
        G4double density = density0 * pow((1 - (kappa) / kappa * M * g0 * h / (R * T)), (1 / (kappa - 1)));
        Air[i] = new G4Material("G4_AIR_" + stri.str(), density, 2);
        Air[i]->AddElement(N, 70 * perCent);
        Air[i]->AddElement(O, 30 * perCent);
    }
}

void MyDetectorConstruction::ConstructAtmosphere()
{
    solidAtmosphere = new G4Box("solidAir", world_hx, world_hy, world_hz / 10.);

    for (G4int i = 0; i < 10; i++)
    {
        logicAtmosphere[i] = new G4LogicalVolume(solidAtmosphere, Air[i], "logicAtmosphere");

        physAtmosphere[i] = new G4PVPlacement(0,
                                              G4ThreeVector(0, 0, world_hz / 10. * 2 * i - world_hz + world_hz / 10.),
                                              logicAtmosphere[i],
                                              "physAtmosphere",
                                              logicWorld,
                                              false,
                                              i,
                                              true);
    }
}