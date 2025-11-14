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
    fs::create_directories(output_dir);
    fs::create_directories(output_dir + "/primaries");
    fs::create_directories(output_dir + "/secondaries");
    fs::create_directories(output_dir + "/backtraced");
    fs::create_directories(output_dir + "/runtime");
    // TODO: tracking of particles
    fs::create_directories(output_dir + "/tracks");

    primaries_output_dir = output_dir + "/primaries/" + project_name;

    // Setting up Timer
    timer = new Timer(output_dir + "/runtime/" + project_name + "_runtime.txt");
    timer->start("Initialization and Run Manager Setup");

    // Set Random seed
    // G4long seed = 345354;
    G4int seed = parser->GetIValue("Seed");
    G4Random::setTheEngine(new CLHEP::RanluxEngine);
    G4Random::setTheSeed(seed, 3);

    // construct the run manager
    runManager = nullptr;
    nThreads = parser->GetIValue("nThreads");
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
        physicsList = new FTFP_BERT();
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

    // visManager = new G4VisExecutive();
    // visManager->Initialize();

    batch_mode = parser->GetBValue("batch_mode");
    timer->stop();
}

G4EarthRunManager::~G4EarthRunManager()
{
    // destructor
    // delete visManager;
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

        merge_output_files();
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
        G4cout << "Density model NRLMSIS_2 initialized." << G4endl;
        densityModel = new NRLMSIS_2(parser->GetSValue("DensityProfileFile"));
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
            output_dir + "/secondaries/" + project_name + "_secondaries" // output directory for sensitive detector
        );
    }
    else if (model_name == "Magnetosphere")
    {
        // TODO: increase the order for better accuracy
        atmosphere = new Magnetosphere(
            parser->GetDimensionedDValue("R_inner"),
            parser->GetDimensionedDValue("R_outer"),
            field,
            output_dir + "/backtraced/" + project_name + "_backtraced");
    }
    else
    {
        G4cerr << "Error: Unsupported atmosphere model: " << model_name << G4endl;
    }
}

void G4EarthRunManager::merge_output_files()
{
    G4cout << "Merging output files..." << G4endl;

    G4String generator_model = parser->GetSValue("GeneratorModel");
    if (generator_model == "SphericalSurfaceSampling" || generator_model == "GenerateFromTable")
    {
        // set up the merged output file
        std::ofstream merged_file(primaries_output_dir + "_primaries.csv", std::ofstream::out | std::ofstream::trunc);

        for (int i = 0; i < nThreads; i++)
        {
            std::string file_to_merge =
                primaries_output_dir + "_primaries_" + std::to_string(i) + ".csv";
            if (fs::exists(file_to_merge))
            {
                std::ifstream src(file_to_merge, std::ios::binary);
                std::ofstream dst(primaries_output_dir + "_primaries.csv", std::ios::binary | std::ios::app);
                dst.seekp(0, std::ios_base::end);
                if (i == 0)
                {
                    // copy header for the first file
                    dst << src.rdbuf();
                }
                else
                {
                    // skip header for other files
                    std::string line;
                    std::getline(src, line); // skip header
                    dst << src.rdbuf();
                }
                src.close();
                dst.close();
                // std::filesystem::remove(file_to_merge); // remove the individual file after merging
            }
            else
            {
                G4cerr << "Warning: File " << file_to_merge << " does not exist." << G4endl;
            }
        }
    }
    else
    {
        G4cout << "Output file merging not supported for generator model: " << generator_model << G4endl;
    }

    G4String atmosphere_model = parser->GetSValue("AtmosphereModel");
    if (atmosphere_model == "SphericalEarth")
    {
        // set up the merged output file
        std::ofstream merged_file(output_dir + "/secondaries/" + project_name + "_secondaries.csv", std::ofstream::out | std::ofstream::trunc);

        for (int i = 0; i < nThreads; i++)
        {
            std::string file_to_merge =
                output_dir + "/secondaries/" + project_name + "_secondaries_" + std::to_string(i) + ".csv";
            if (fs::exists(file_to_merge))
            {
                std::ifstream src(file_to_merge, std::ios::binary);
                std::ofstream dst(output_dir + "/secondaries/" + project_name + "_secondaries.csv", std::ios::binary | std::ios::app);
                dst.seekp(0, std::ios_base::end);
                if (i == 0)
                {
                    // copy header for the first file
                    dst << src.rdbuf();
                }
                else
                {
                    // skip header for other files
                    std::string line;
                    std::getline(src, line); // skip header
                    dst << src.rdbuf();
                }
                src.close();
                dst.close();
                // std::filesystem::remove(file_to_merge); // remove the individual file after merging
            }
            else
            {
                G4cerr << "Warning: File " << file_to_merge << " does not exist." << G4endl;
            }
        }
    }
    else if (atmosphere_model == "Magnetosphere")
    {
        // set up the merged output file
        std::ofstream merged_file(output_dir + "/backtraced/" + project_name + "_backtraced.csv", std::ofstream::out | std::ofstream::trunc);

        for (int i = 0; i < nThreads; i++)
        {
            std::string file_to_merge =
                output_dir + "/backtraced/" + project_name + "_backtraced_" + std::to_string(i) + ".csv";
            if (fs::exists(file_to_merge))
            {
                std::ifstream src(file_to_merge, std::ios::binary);
                std::ofstream dst(output_dir + "/backtraced/" + project_name + "_backtraced.csv", std::ios::binary | std::ios::app);
                dst.seekp(0, std::ios_base::end);
                if (i == 0)
                {
                    // copy header for the first file
                    dst << src.rdbuf();
                }
                else
                {
                    // skip header for other files
                    std::string line;
                    std::getline(src, line); // skip header
                    dst << src.rdbuf();
                }
                src.close();
                dst.close();
                // std::filesystem::remove(file_to_merge); // remove the individual file after merging
            }
            else
            {
                G4cerr << "Warning: File " << file_to_merge << " does not exist." << G4endl;
            }
        }
    }
    else
    {
        G4cout << "Output file merging not supported for atmosphere model: " << atmosphere_model << G4endl;
    }
}