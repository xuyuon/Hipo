#include "runManager.hh"

G4EarthRunManager::G4EarthRunManager(G4String config_file)
{
    this->config_file = config_file;

    // Setting Up Parser
    parser = new Parser();
    parser->ReadConfig(config_file);
    // Read project name
    project_name = parser->GetSValue("ProjectName");
    nThreads = parser->GetIValue("nThreads");
    nSensLayer = parser->GetIValue("nSensitiveLayer");

    // Create file manager
    output_dir = parser->GetSValue("OutputDir");
    fileManager = new FileManager(project_name,
                                  output_dir,
                                  parser->GetSValue("DetectingParticleName"),
                                  nThreads,
                                  nSensLayer);

    primaries_output_dir = output_dir + "/primaries/" + project_name;

    // Setting up Timer
    timer = new Timer(output_dir + "/runtime/" + project_name + "_runtime.txt");
    timer->start("Initialization and Run Manager Setup");

    // Set Random seed
    G4int seed = parser->GetIValue("Seed");
    G4Random::setTheEngine(new CLHEP::RanluxEngine);
    G4Random::setTheSeed(seed, 3);

    // construct the run manager
    runManager = nullptr;
    if (nThreads > 1)
    {
        runManager = new G4MTRunManager();
        runManager->SetNumberOfThreads(nThreads);
    }
    else
    {
        runManager = new G4RunManager();
    }

    // set physics list
    if (parser->GetBValue("Enable_interaction") == false)
    {
        G4cout << "Interaction physics disabled." << G4endl;
        physicsList = new MagneticTransport();
    }
    else
    {
        G4cout << "Interaction physics enabled." << G4endl;
        physicsList = new FTFP_BERT_Physics();
    }
    runManager->SetUserInitialization(physicsList);

    init_density_model();
    init_field();
    init_atmosphere();

    // generator is initialized in ActionInitialization
    action = new ActionInitialization(parser, primaries_output_dir);
    runManager->SetUserInitialization(atmosphere);
    runManager->SetUserInitialization(action);

    // initialize G4 kernel
    runManager->Initialize();

    ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    UImanager = G4UImanager::GetUIpointer();

    batch_mode = parser->GetBValue("batch_mode");
    timer->stop();
}

G4EarthRunManager::~G4EarthRunManager()
{
    // destructor
    delete runManager;
}

void G4EarthRunManager::run()
{
    if (batch_mode)
    {
        // batch mode
        timer->start("Batch Mode Execution");
        G4cout << "Starting Batch Execution" << G4endl;

        // when the generator does not specify the number of events,
        // use the nEvents from the config file
        if (action->get_n_events() == -1)
        {
            G4cout << parser->GetIValue("nEvents") << " events will be generated." << G4endl;
            runManager->BeamOn(parser->GetIValue("nEvents"));
        }
        else
        {
            // otherwise use the nEvents from the generator
            G4cout << action->get_n_events() << " events will be generated." << G4endl;
            runManager->BeamOn(action->get_n_events());
        }

        G4cout << "Batch Execution Completed" << G4endl;
        timer->stop();

        // merge output files
        fileManager->merge_all_output_files();
    }
    else
    {
        // interactive mode
        G4cerr << "Interactive mode not supported." << G4endl;
    }
}

void G4EarthRunManager::init_density_model()
{
    G4String model_name = parser->GetSValue("DensityModel");
    if (model_name == "TabulatedDensity")
    {
        G4cout << "Density model TabulatedDensity initialized." << G4endl;
        densityModel = new TabulatedDensity(
            parser->GetDimensionedDValue("r_min"),
            parser->GetDimensionedDValue("r_pause"),
            parser->GetDimensionedDValue("r_max"),
            parser->GetDimensionedDValue("delta_r1"),
            parser->GetDimensionedDValue("delta_r2"),
            parser->GetDimensionedDValue("delta_theta"),
            parser->GetDimensionedDValue("delta_phi"),
            parser->GetSValue("DensityProfileFile"));
    }
    else if (model_name == "None")
    {
        G4cout << "No density model selected." << G4endl;
        densityModel = nullptr;
    }
    else
    {
        G4cerr << "Error: Unsupported density model: " << model_name << G4endl;
    }
}

void G4EarthRunManager::init_field()
{
    G4String model_name = parser->GetSValue("FieldModel");
    if (model_name == "TabulatedEarthMagneticField")
    {
        field = new TabulatedEarthMagneticField(
            parser->GetSValue("FieldTableFile"));
    }
    else if (model_name == "IGRF")
    {
        field = new IGRF(
            parser->GetSValue("IGRFDataFile"),
            parser->GetIValue("Year"),
            parser->GetIValue("Month"),
            parser->GetIValue("Day"),
            parser->GetIValue("n_order_max"));
    }
    else if (model_name == "TA16")
    {
        field = new TA16(
            parser->GetIValue("Year"),
            parser->GetIValue("Month"),
            parser->GetIValue("Day"),
            parser->GetDValue("pdyn"),
            parser->GetDValue("symhc"),
            parser->GetDValue("xind"),
            parser->GetDValue("by_imf"),
            parser->GetDimensionedDValue("ps"),
            parser->GetDValue("vxgse"),
            parser->GetDValue("vygse"),
            parser->GetDValue("vzgse"),
            parser->GetSValue("TA16ParamFile"),
            parser->GetSValue("IGRFDataFile"),
            parser->GetIValue("n_order_max"));
    }
    else if (model_name == "NoField")
    {
        field = nullptr;
    }
    else
    {
        G4cerr << "Error: Unsupported field model: " << model_name << G4endl;
    }
}

void G4EarthRunManager::init_atmosphere()
{
    // Must be called after init_density_model(), init_field()
    G4String model_name = parser->GetSValue("AtmosphereModel");
    std::vector<G4double> R_sens;
    for (int i = 1; i <= nSensLayer; i++)
    {
        R_sens.push_back(parser->GetDimensionedDValue("R_sens_" + std::to_string(i)));
    }
    
    if (model_name == "SphericalEarth")
    {
        G4double epsilon = 1e-6 * m; // small offset to avoid numerical issues

        atmosphere = new SphericalEarth(
            densityModel, // density model
            R_sens,       // h_sens
            parser->GetDimensionedDValue("R_earth"),
            field,
            parser->GetSValue("DetectingParticleName"),
            fileManager);
    }
    else if (model_name == "Magnetosphere")
    {
        atmosphere = new Magnetosphere(
            parser->GetDimensionedDValue("R_inner"),
            parser->GetDimensionedDValue("R_outer"),
            R_sens,
            field,
            parser->GetSValue("DetectingParticleName"),
            fileManager);
    }
    else
    {
        G4cerr << "Error: Unsupported atmosphere model: " << model_name << G4endl;
    }
}