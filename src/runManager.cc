#include "runManager.hh"

G4EarthRunManager::G4EarthRunManager(G4String config_file)
{
    this->config_file = config_file;

    // Setting Up Parser
    parser = new Parser();
    parser->ReadConfig(config_file);

    // Read project name
    project_name = parser->GetSValue("ProjectName");

    // Create output directories
    output_dir = parser->GetSValue("OutputDir");
    G4cout << "Output directory: " << output_dir << G4endl;
    std::filesystem::create_directories(output_dir);
    std::filesystem::create_directories(output_dir + "/primaries");
    std::filesystem::create_directories(output_dir + "/secondaries");
    std::filesystem::create_directories(output_dir + "/backtraced");
    std::filesystem::create_directories(output_dir + "/runtime");

    // Setting up Timer
    timer = new Timer(output_dir + "/runtime/" + project_name + "_runtime.txt");
    timer->start("Initialization and Run Manager Setup");

    // Set Random seed
    G4long seed = 345354;
    G4Random::setTheEngine(new CLHEP::RanluxEngine);
    G4Random::setTheSeed(seed, 3);

    // construct the run manager
    runManager = nullptr;
    G4int nThreads = parser->GetIValue("nThreads");
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
    physicsList = new FTFP_BERT();
    runManager->SetUserInitialization(physicsList);

    init_density_model();
    init_field();
    init_atmosphere();
    init_generator();
    action = new ActionInitialization(generator);
    runManager->SetUserInitialization(atmosphere);
    runManager->SetUserInitialization(action);

    // initialize G4 kernel
    runManager->Initialize();

    ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    UImanager = G4UImanager::GetUIpointer();

    visManager = new G4VisExecutive();
    visManager->Initialize();

    batch_mode = parser->GetBValue("batch_mode");
    timer->stop();
}

G4EarthRunManager::~G4EarthRunManager()
{
    // destructor
}

void G4EarthRunManager::run()
{
    if (batch_mode)
    {
        // batch mode
        timer->start("Batch Mode Execution");
        G4String command = "/control/execute ";
        G4String fileName = parser->GetSValue("macro_file");
        UImanager->ApplyCommand(command + fileName);
        timer->stop();
    }
    else
    {
        // interactive mode
        G4cerr << "Interactive mode not supported yet." << G4endl;
        // ui = new G4UIExecutive(argc, argv);
        // UImanager->ApplyCommand("/control/execute vis.mac");
        // ui->SessionStart();
        // delete ui;
    }
}

void G4EarthRunManager::init_density_model()
{
    G4String model_name = parser->GetSValue("DensityModel");
    if (model_name == "NRLMSIS_2")
    {
        densityModel = new NRLMSIS_2(parser->GetSValue("DensityProfileFile"));
    }
    else if (model_name == "None")
    {
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
    else if (model_name == "EarthMagneticField")
    {
        field = new EarthMagneticField(
            parser->GetSValue("IGRFDataFile"),
            parser->GetIValue("Year"),
            parser->GetIValue("Month"),
            parser->GetIValue("Day"),
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
    if (model_name == "SphericalEarth")
    {
        G4double epsilon = 1e-6 * m; // small offset to avoid numerical issues
        atmosphere = new SphericalEarth(
            densityModel,                                         // density model
            parser->GetDimensionedDValue("h_detector") + epsilon, // h_sens
            parser->GetDimensionedDValue("R_earth"),
            field,
            output_dir + "/secondaries/" + project_name + "_secondaries.csv" // output directory for sensitive detector
        );
    }
    else if (model_name == "Magnetosphere")
    {
        // TODO: increase the order for better accuracy
        atmosphere = new Magnetosphere(
            parser->GetDimensionedDValue("R_inner"),
            parser->GetDimensionedDValue("R_outer"),
            field,
            output_dir + "/backtraced/" + project_name + "_backtraced.csv"
        );
    }
    else
    {
        G4cerr << "Error: Unsupported atmosphere model: " << model_name << G4endl;
    }
}

void G4EarthRunManager::init_generator()
{
    G4String model_name = parser->GetSValue("GeneratorModel");
    if (model_name == "SphericalSurfaceSampling")
    {
        generator = new SphericalSurfaceSampling(
            parser->GetSValue("ParticleName"),
            get_sampler(parser->GetSValue("EnergySpectrum"), "EnergySpectrum"),
            get_coordinates_sampler(parser->GetSValue("InitialPosition"), "InitialPosition"),
            get_sampler(parser->GetSValue("NadirAngle"), "NadirAngle"),
            get_sampler(parser->GetSValue("AzimuthAngle"), "AzimuthAngle"),
            output_dir + "/primaries/" + project_name + "_primaries.csv" // output directory for sensitive detector
        );
    }
    else
    {
        G4cerr << "Error: Unsupported generator model: " << model_name << G4endl;
    }
}

SamplingBase *G4EarthRunManager::get_sampler(G4String sampler_name, G4String label) const
{
    if (sampler_name == "LogUniform")
    {
        return new LogUniform(parser->GetDimensionedDValue(label + "_min"), parser->GetDimensionedDValue(label + "_max"));
    }
    else if (sampler_name == "Uniform")
    {
        return new Uniform(parser->GetDimensionedDValue(label + "_min"), parser->GetDimensionedDValue(label + "_max"));
    }
    else if (sampler_name == "Sine")
    {
        return new Sine();
    }
    else if (sampler_name == "Scaled")
    {
        SamplingBase *base_sampler = get_sampler(parser->GetSValue(label + "_base"), label + "_base");
        G4double scale_factor = parser->GetDValue(label + "_scale");
        return new Scaled(base_sampler, scale_factor);
    }
    else if (sampler_name == "Offset")
    {
        SamplingBase *base_sampler = get_sampler(parser->GetSValue(label + "_base"), label + "_base");
        G4double offset = parser->GetDValue(label + "_offset");
        return new Offset(base_sampler, offset);
    }
    else
    {
        G4cerr << "Error: Unsupported sampler: " << sampler_name << G4endl;
        return nullptr;
    }
}

CoordinatesSamplingBase *G4EarthRunManager::get_coordinates_sampler(G4String sampler_name, G4String label) const
{
    if (sampler_name == "UniformSphericalSurface")
    {
        return new UniformSphericalSurface(parser->GetDimensionedDValue(label + "_mag"));
    }
    else
    {
        G4cerr << "Error: Unsupported coordinates sampler: " << sampler_name << G4endl;
        return nullptr;
    }
}