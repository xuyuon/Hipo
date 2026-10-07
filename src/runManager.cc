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

    // Initialize file manager
    output_dir = parser->GetSValue("OutputDir");
    outputManager = new OutputManager();
    outputManager->initialize(project_name,
                            output_dir,
                            parser->GetSValue("DetectingParticleName"),
                            nThreads,
                            nSensLayer);

    // Setting up Timer
    timer = new Timer(output_dir + "/runtime/" + project_name + "_runtime.txt");
    timer->start("Initialization and Run Manager Setup");

    // Disable GEANT4 output if necessary
    if(parser->GetBValue("disable_geant4_verbose"))
    {
        disable_stdout();
    }

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

    // initialize different physics models
    init_density_model();
    init_field();
    init_atmosphere();
    this->n_events = get_n_events();

    // generator is initialized in ActionInitialization
    action = new ActionInitialization(parser, outputManager);
    runManager->SetUserInitialization(atmosphere);
    runManager->SetUserInitialization(action);

    // initialize G4 kernel
    runManager->Initialize();

    ui = nullptr;
    // get the pointer to the UI manager and set verbosities
    UImanager = G4UImanager::GetUIpointer();

    batch_mode = parser->GetBValue("batch_mode");
    if(parser->GetBValue("disable_geant4_verbose"))
    {
        enable_stdout();
    }
    timer->stop();
}

G4EarthRunManager::~G4EarthRunManager()
{
    // destructor
    delete runManager;
    delete parser;
    delete outputManager;
    delete timer;
    delete densityModel;
    delete field;
}

void G4EarthRunManager::run()
{
    if (batch_mode)
    {
        // batch mode
        timer->start("Batch Mode Execution");
        G4cout << "Starting Batch Execution" << G4endl;

        // Generate events
        G4cout << n_events << " events will be generated." << G4endl;
        runManager->BeamOn(n_events);
        G4cout << "Batch Execution Completed" << G4endl;
        timer->stop();

        // merge output files
        outputManager->merge_all_output_files();
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
            outputManager);
    }
    else if (model_name == "Magnetosphere")
    {
        atmosphere = new Magnetosphere(
            parser->GetDimensionedDValue("R_inner"),
            parser->GetDimensionedDValue("R_outer"),
            R_sens,
            field,
            parser->GetSValue("DetectingParticleName"),
            outputManager);
    }
    else
    {
        G4cerr << "Error: Unsupported atmosphere model: " << model_name << G4endl;
    }
}

int G4EarthRunManager::get_n_events()
{
    G4String model_name = parser->GetSValue("GeneratorModel");

    if (model_name == "SphericalSurfaceSampling")
    {
        return parser->GetIValue("nEvents");
    }
    else if (model_name == "GenerateFromTable")
    {
        // Read the data file
        std::string data_file = parser->GetSValue("PrimariesDataFile");
        std::ifstream file(data_file);
        if (!file.is_open())
        {
            G4cerr << "Error: Could not open data file " << data_file << G4endl;
            return -1;
        }
        std::string line;
        // Read the first line to get the header information
        std::getline(file, line);

        int count = 0;
        while (std::getline(file, line))
        {
            // Count non-empty lines
            if (line.find_first_not_of(" \t\r\n") != std::string::npos)
            {
                count++;
            }
        }
        file.close();
    }
    else
    {
        G4cerr << "Error: Unsupported generator model: " << model_name << G4endl;
        return -1;
    }
    return -1;
}

void G4EarthRunManager::disable_stdout()
{
    FILE* tmp = fopen("/dev/null", "a+");

    fflush(stdout);

    outfd = dup(STDOUT_FILENO);

    if (outfd == -1 || dup2(fileno(tmp), STDOUT_FILENO) == -1)
        errx(1, "failed to disable stdout");

    fclose(tmp);
}

void G4EarthRunManager::enable_stdout()
{
    fflush(stdout);

    if (dup2(outfd, STDOUT_FILENO) == -1)
        errx(1, "failed to enable stdout");

    close(outfd);
}
