#include "atmosphereModel.hh"

SphericalEarth::SphericalEarth(DensityModelBase *densityModel,
                               std::vector<G4double> R_sens,
                               G4double R_earth,
                               G4MagneticField *Bfield,
                               std::string particle_name,
                               FileManager *fileManager)
{
    this->R_earth = R_earth; // Global average number
    this->R_sens = R_sens;   // radial position of the sensitive layer
    this->particle_name = particle_name;
    this->nSensLayers = R_sens.size();
    sens_layer.resize(nSensLayers, -1);

    delta_sens = 1. * cm; // thickness of the sensitive layer

    // Set maximum step length to avoid tracking looping particles
    G4double MaxTrackLength = 2 * 3.141592653589793 * (2 * R_earth);
    G4double MaxStepSize = 10 * km; // maximum step size of 10 km
    userLimits = new G4UserLimits();
    userLimits->SetUserMaxTrackLength(MaxTrackLength);
    userLimits->SetMaxAllowedStep(MaxStepSize);

    // Initialize density model
    this->densityModel = densityModel;

    this->Bfield = Bfield;
    this->fileManager = fileManager;

    nLayers_r = densityModel->Get_r_boundaries_size() - 1;         // Number of altitude layers
    nLayers_theta = densityModel->Get_theta_boundaries_size() - 1; // Number of latitude layers
    nLayers_phi = densityModel->Get_phi_boundaries_size() - 1;     // Number of longitude layers
}

SphericalEarth::~SphericalEarth()
{
    // Destructor
}

G4VPhysicalVolume *SphericalEarth::Construct()
{
    // Define world material
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_Galactic");

    // Create the world volume
    solidWorld = new G4Sphere("solidWorld",
                              R_earth,
                              densityModel->Get_r(densityModel->Get_r_boundaries_size() - 1),
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

    // // Set world region
    // G4Region *worldRegion = new G4Region("WorldRegion");
    // worldRegion->AddRootLogicalVolume(logicWorld);

    // // Assign user limits to all logical volumes in the world region
    // worldRegion->SetUserLimits(userLimits);
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

    G4double epsilon = 1e-6 * m; // Small value to prevent volume overlap

    Air.resize(nLayers_r, std::vector<std::vector<G4Material *>>(nLayers_theta, std::vector<G4Material *>(nLayers_phi, nullptr)));
    solidAtmosphere.resize(nLayers_r, std::vector<std::vector<G4Sphere *>>(nLayers_theta, std::vector<G4Sphere *>(nLayers_phi, nullptr)));
    logicAtmosphere.resize(nLayers_r, std::vector<std::vector<G4LogicalVolume *>>(nLayers_theta, std::vector<G4LogicalVolume *>(nLayers_phi, nullptr)));
    physAtmosphere.resize(nLayers_r, std::vector<std::vector<G4VPhysicalVolume *>>(nLayers_theta, std::vector<G4VPhysicalVolume *>(nLayers_phi, nullptr)));

    for (G4int i = 0; i < nLayers_r; ++i)
    {
        for (G4int j = 0; j < nLayers_theta; ++j)
        {
            for (G4int k = 0; k < nLayers_phi; ++k)
            {
                Air[i][j][k] = new G4Material("Air_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k),
                                              densityModel->GetDensities(i, j, k), 5);
                Air[i][j][k]->AddElement(N, densityModel->GetNPercentageMass(i, j, k));
                Air[i][j][k]->AddElement(O, densityModel->GetOPercentageMass(i, j, k));
                Air[i][j][k]->AddElement(Ar, densityModel->GetArPercentageMass(i, j, k));
                Air[i][j][k]->AddElement(He, densityModel->GetHePercentageMass(i, j, k));
                Air[i][j][k]->AddElement(H, densityModel->GetHPercentageMass(i, j, k));

                // Set the sensitive layer index
                for (int l = 0; l < nSensLayers; l++)
                {
                    if (R_sens[l] > densityModel->Get_r(i) && R_sens[l] < densityModel->Get_r(i + 1))
                    {
                        sens_layer[l] = i;
                    }
                }

                solidAtmosphere[i][j][k] = new G4Sphere("solidAtmosphere_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k),
                                                        densityModel->Get_r(i),                                       // inner radius
                                                        densityModel->Get_r(i + 1),                                   // outer radius
                                                        densityModel->Get_phi(k),                                     // starting phi
                                                        densityModel->Get_phi(k + 1) - densityModel->Get_phi(k),      // delta phi
                                                        densityModel->Get_theta(j),                                   // starting theta
                                                        densityModel->Get_theta(j + 1) - densityModel->Get_theta(j)); // delta theta
                logicAtmosphere[i][j][k] = new G4LogicalVolume(solidAtmosphere[i][j][k], Air[i][j][k], "logicAtmosphere");
                logicAtmosphere[i][j][k]->SetUserLimits(userLimits);
                physAtmosphere[i][j][k] = new G4PVPlacement(0,
                                                            G4ThreeVector(0., 0., 0.),
                                                            logicAtmosphere[i][j][k],
                                                            "physAtmosphere",
                                                            logicWorld,
                                                            false,
                                                            i * nLayers_theta * nLayers_phi + j * nLayers_phi + k,
                                                            true);
            }
        }
    }
}

void SphericalEarth::ConstructSensitiveLayer()
{
    // Create the sensitive layer as a thin spherical shell
    solidSensLayer.resize(nSensLayers, std::vector<std::vector<G4Sphere *>>(nLayers_theta, std::vector<G4Sphere *>(nLayers_phi, nullptr)));
    logicSensLayer.resize(nSensLayers, std::vector<std::vector<G4LogicalVolume *>>(nLayers_theta, std::vector<G4LogicalVolume *>(nLayers_phi, nullptr)));
    physSensLayer.resize(nSensLayers, std::vector<std::vector<G4VPhysicalVolume *>>(nLayers_theta, std::vector<G4VPhysicalVolume *>(nLayers_phi, nullptr)));

    for (int l = 0; l < nSensLayers; l++)
    {
        // Check if the sensitive layer is within a layer
        if (R_sens[l] < densityModel->Get_r(0) || R_sens[l] > densityModel->Get_r(nLayers_r - 1))
        {
            G4cerr << "Error: Sensitive layer should be within the atmosphere layers." << G4endl;
        }

        for (G4int j = 0; j < nLayers_theta; ++j)
        {
            for (G4int k = 0; k < nLayers_phi; ++k)
            {
                solidSensLayer[l][j][k] = new G4Sphere("solidSensLayer_" + std::to_string(l) + "_" + std::to_string(j) + "_" + std::to_string(k),
                                                       R_sens[l],
                                                       R_sens[l] + delta_sens,
                                                       densityModel->Get_phi(k),                                     // starting phi
                                                       densityModel->Get_phi(k + 1) - densityModel->Get_phi(k),      // delta phi
                                                       densityModel->Get_theta(j),                                   // starting theta
                                                       densityModel->Get_theta(j + 1) - densityModel->Get_theta(j)); // delta theta
                logicSensLayer[l][j][k] = new G4LogicalVolume(solidSensLayer[l][j][k],
                                                              Air[sens_layer[l]][j][k],
                                                              "logicSensLayer_" + std::to_string(l) + "_" + std::to_string(j) + "_" + std::to_string(k));
                physSensLayer[l][j][k] = new G4PVPlacement(0,
                                                           G4ThreeVector(0., 0., 0.),
                                                           logicSensLayer[l][j][k],
                                                           "physSensLayer_" + std::to_string(l) + "_" + std::to_string(j) + "_" + std::to_string(k),
                                                           logicAtmosphere[sens_layer[l]][j][k],
                                                           false,
                                                           j * nLayers_phi + k,
                                                           true);
            }
        }
    }
}

void SphericalEarth::ConstructSDandField()
{
    // Construct sensitive detectors
    for (int l = 0; l < nSensLayers; l++)
    {
        SensitiveDetectorBase *sd = new SphericalShellSD("SensitiveDetector_" + std::to_string(l),
                                                         particle_name,
                                                         fileManager,
                                                         l);
        for (G4int j = 0; j < nLayers_theta; ++j)
        {
            for (G4int k = 0; k < nLayers_phi; ++k)
            {
                logicSensLayer[l][j][k]->SetSensitiveDetector(sd);
            }
        }
    }

    // Construct magnetic field
    if (Bfield != nullptr)
    {
        G4double minStep = 1.0 * km; // minimal step of 1 km
        G4double minEps = 1e-5;      //   Minimum & value for smallest steps
        G4double maxEps = 1e-3;
        G4bool allLocal = true;
        G4double deltaStep = minStep;
        G4double deltaChord = minStep;
        G4double deltaIntersection = minStep;

        // Create an equation of motion for this field
        G4Mag_UsualEqRhs *fEquation = new G4Mag_UsualEqRhs(Bfield);

        // Create the Runge-Kutta 'stepper' using the efficient 'DoPri5' method
        G4MagIntegratorStepper *fstepper = new G4DormandPrince745(fEquation);

        // Set this field to the global field manager
        G4FieldManager *globalFieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
        globalFieldMgr->SetDetectorField(Bfield);

        G4MagInt_Driver *pIntgrDriver = new G4MagInt_Driver(minStep, fstepper);
        G4ChordFinder *pChordFinder = new G4ChordFinder(pIntgrDriver);
        globalFieldMgr->SetChordFinder(pChordFinder);

        globalFieldMgr->GetChordFinder()->SetDeltaChord(deltaChord);
        globalFieldMgr->SetAccuraciesWithDeltaOneStep(deltaStep);
        globalFieldMgr->SetMinimumEpsilonStep(minEps);
        globalFieldMgr->SetMaximumEpsilonStep(maxEps);
        globalFieldMgr->SetDeltaOneStep(deltaStep);
        globalFieldMgr->SetDeltaIntersection(deltaIntersection);
    }
}

Magnetosphere::Magnetosphere(G4double R_inner,
                             G4double R_outer,
                             std::vector<G4double> R_sens,
                             G4MagneticField *Bfield,
                             std::string particle_name,
                             FileManager *fileManager)
{
    // User-defined variables
    this->R_inner = R_inner;
    this->R_outer = R_outer;
    this->R_sens = R_sens;
    this->Bfield = Bfield;
    this->particle_name = particle_name;
    this->fileManager = fileManager;

    if (R_sens.size() != 1)
    {
        G4cerr << "Error: Only one sensitive layer is supported in the Magnetosphere model." << G4endl;
    }

    // Set maximum step length to avoid tracking looping particles
    G4double MaxTrackLength = 2 * 3.141592653589793 * (5 * R_outer);
    G4double MaxStepSize = 10 * km; // maximum step size of 10 km
    userLimits = new G4UserLimits();
    userLimits->SetUserMaxTrackLength(MaxTrackLength);
    userLimits->SetMaxAllowedStep(MaxStepSize);
}

Magnetosphere::~Magnetosphere()
{
    // Destructor
}

G4VPhysicalVolume *Magnetosphere::Construct()
{
    // Define world material
    // Density of G4_Galactic: 1.0e-25 [g/cm3])
    G4NistManager *nist = G4NistManager::Instance();
    worldMat = nist->FindOrBuildMaterial("G4_Galactic");

    // Create the world volume
    solidWorld = new G4Sphere("solidWorld",
                              R_inner,
                              R_outer,
                              0 * degree,
                              360 * degree,
                              0. * degree,
                              180. * degree);
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    logicWorld->SetUserLimits(userLimits);
    physWorld = new G4PVPlacement(0,
                                  G4ThreeVector(0., 0., 0.),
                                  logicWorld,
                                  "physWorld",
                                  0,
                                  false,
                                  0,
                                  true);
    ConstructSensitiveLayer();
    return physWorld;
}

void Magnetosphere::ConstructSDandField()
{
    // Construct sensitive detectors that record anti-protons
    SensitiveDetectorBase *sd = new SphericalShellSD("SensitiveDetector",
                                                     particle_name,
                                                     fileManager,
                                                     0);
    logicSensLayer->SetSensitiveDetector(sd);

    // Construct magnetic field
    if (Bfield != nullptr)
    {
        G4double minStep = 1.0 * km; // minimal step of 1 km
        G4double minEps = 1e-5;      //   Minimum & value for smallest steps
        G4double maxEps = 1e-3;      //   Minimum & value for largest steps
        G4bool allLocal = true;
        G4double deltaStep = minStep;
        G4double deltaChord = minStep;
        G4double deltaIntersection = minStep;

        // Create an equation of motion for this field
        G4Mag_UsualEqRhs *fEquation = new G4Mag_UsualEqRhs(Bfield);

        // Create the Runge-Kutta 'stepper' using the efficient 'DoPri5' method
        G4MagIntegratorStepper *fstepper = new G4DormandPrince745(fEquation);

        // Set this field to the global field manager
        G4FieldManager *globalFieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
        globalFieldMgr->SetDetectorField(Bfield);

        G4MagInt_Driver *pIntgrDriver = new G4MagInt_Driver(minStep, fstepper);
        G4ChordFinder *pChordFinder = new G4ChordFinder(pIntgrDriver);
        globalFieldMgr->SetChordFinder(pChordFinder);

        globalFieldMgr->GetChordFinder()->SetDeltaChord(deltaChord);
        globalFieldMgr->SetAccuraciesWithDeltaOneStep(deltaStep);
        globalFieldMgr->SetMinimumEpsilonStep(minEps);
        globalFieldMgr->SetMaximumEpsilonStep(maxEps);
        globalFieldMgr->SetDeltaOneStep(deltaStep);
        globalFieldMgr->SetDeltaIntersection(deltaIntersection);
    }
}

void Magnetosphere::ConstructSensitiveLayer()
{
    //
    G4double epsilon = 1e-6 * m;  // small offset to avoid numerical issues
    G4double delta_sens = 1. * m; // thickness of the sensitive layer
    solidSensLayer = new G4Sphere(
        "solidSensLayer",
        R_sens[0] + epsilon,
        R_sens[0] + delta_sens - epsilon,
        0 * degree,
        360 * degree,
        0. * degree,
        180. * degree);
    logicSensLayer = new G4LogicalVolume(solidSensLayer,
                                         worldMat,
                                         "logicSensLayer");
    physSensLayer = new G4PVPlacement(0,
                                      G4ThreeVector(0., 0., 0.),
                                      logicSensLayer,
                                      "physSensLayer",
                                      logicWorld,
                                      false,
                                      0,
                                      true);
}
