#include "runManager.hh"

#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

G4EarthRunManager::G4EarthRunManager(G4String config_file)
{
    this->config_file = config_file;

    parser = new Parser();
    if (!parser->ReadConfig(config_file))
    {
        throw std::runtime_error("Failed to read config file: " + std::string(config_file));
    }
    validate_config_schema();

    project_name = parser->GetSValue("ProjectName");
    nThreads = parser->GetIValue("nThreads");
    sensitive_layers = resolve_sensitive_layers();

    output_dir = parser->GetSValue("OutputDir");
    fileManager = new FileManager(project_name,
                                  output_dir,
                                  parser->GetSValue("DetectingParticleName"),
                                  nThreads,
                                  static_cast<int>(sensitive_layers.size()));

    primaries_output_dir = output_dir + "/primaries/" + project_name;

    timer = new Timer(output_dir + "/runtime/" + project_name + "_runtime.txt");
    timer->start("Initialization and Run Manager Setup");

    G4int seed = parser->GetIValue("Seed");
    G4Random::setTheEngine(new CLHEP::RanluxEngine);
    G4Random::setTheSeed(seed, 3);

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

    if (!parser->GetBValue("Enable_interaction"))
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

    action = new ActionInitialization(parser, primaries_output_dir);
    runManager->SetUserInitialization(atmosphere);
    runManager->SetUserInitialization(action);
    runManager->Initialize();

    ui = nullptr;
    UImanager = G4UImanager::GetUIpointer();
    batch_mode = parser->GetBValue("batch_mode");

    write_run_metadata();
    timer->stop();
}

G4EarthRunManager::~G4EarthRunManager()
{
    delete runManager;
}

void G4EarthRunManager::validate_sampler(const std::string &label, std::vector<std::string> &errors, int depth)
{
    if (depth > 3)
    {
        errors.push_back("Sampler nesting too deep for label: " + label);
        return;
    }

    if (!parser->HasKey(label))
    {
        errors.push_back("Missing sampler key: " + label);
        return;
    }

    const std::string sampler_name = parser->GetSValue(label);
    if (sampler_name == "LogUniform" || sampler_name == "Uniform")
    {
        if (!parser->HasKey(label + "_min") || !parser->HasKey(label + "_max"))
        {
            errors.push_back("Sampler " + label + " requires keys " + label + "_min and " + label + "_max");
        }
    }
    else if (sampler_name == "Scaled")
    {
        if (!parser->HasKey(label + "_base") || !parser->HasKey(label + "_scale"))
        {
            errors.push_back("Sampler " + label + " requires keys " + label + "_base and " + label + "_scale");
            return;
        }
        validate_sampler(label + "_base", errors, depth + 1);
    }
    else if (sampler_name == "Offset")
    {
        if (!parser->HasKey(label + "_base") || !parser->HasKey(label + "_offset"))
        {
            errors.push_back("Sampler " + label + " requires keys " + label + "_base and " + label + "_offset");
            return;
        }
        validate_sampler(label + "_base", errors, depth + 1);
    }
    else if (sampler_name != "Sine" && sampler_name != "Lambertian" && sampler_name != "UpwardLambertian")
    {
        errors.push_back("Unsupported sampler " + sampler_name + " for label " + label);
    }
}

void G4EarthRunManager::validate_config_schema()
{
    std::vector<std::string> errors;

    const std::vector<std::string> required_base = {
        "ProjectName", "nThreads", "batch_mode", "OutputDir", "Enable_interaction", "Seed",
        "DensityModel", "FieldModel", "AtmosphereModel", "GeneratorModel", "ParticleName", "DetectingParticleName"};

    for (const auto &key : required_base)
    {
        if (!parser->HasKey(key))
        {
            errors.push_back("Missing required key: " + key);
        }
    }

    if (parser->HasKey("nThreads") && parser->GetIValue("nThreads") < 1)
    {
        errors.push_back("nThreads must be >= 1");
    }

    const std::string density_model = parser->HasKey("DensityModel") ? parser->GetSValue("DensityModel") : "";
    if (density_model == "TabulatedDensity")
    {
        for (const auto &key : {"r_min", "r_pause", "r_max", "delta_r1", "delta_r2", "delta_theta", "delta_phi", "DensityProfileFile"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("DensityModel=TabulatedDensity requires key: " + std::string(key));
            }
        }
    }
    else if (density_model != "None")
    {
        errors.push_back("Unsupported DensityModel: " + density_model);
    }

    const std::string field_model = parser->HasKey("FieldModel") ? parser->GetSValue("FieldModel") : "";
    if (field_model == "TabulatedEarthMagneticField")
    {
        if (!parser->HasKey("FieldTableFile"))
        {
            errors.push_back("FieldModel=TabulatedEarthMagneticField requires FieldTableFile");
        }
    }
    else if (field_model == "IGRF")
    {
        for (const auto &key : {"IGRFDataFile", "Year", "Month", "Day", "n_order_max"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("FieldModel=IGRF requires key: " + std::string(key));
            }
        }
    }
    else if (field_model == "TA16")
    {
        for (const auto &key : {"Year", "Month", "Day", "pdyn", "symhc", "xind", "by_imf", "ps", "vxgse", "vygse", "vzgse", "TA16ParamFile", "IGRFDataFile", "n_order_max"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("FieldModel=TA16 requires key: " + std::string(key));
            }
        }
    }
    else if (field_model != "NoField")
    {
        errors.push_back("Unsupported FieldModel: " + field_model);
    }

    const std::string atmosphere_model = parser->HasKey("AtmosphereModel") ? parser->GetSValue("AtmosphereModel") : "";
    if (atmosphere_model == "SphericalEarth")
    {
        if (!parser->HasKey("R_earth"))
        {
            errors.push_back("AtmosphereModel=SphericalEarth requires R_earth");
        }
    }
    else if (atmosphere_model == "Magnetosphere")
    {
        for (const auto &key : {"R_inner", "R_outer"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("AtmosphereModel=Magnetosphere requires key: " + std::string(key));
            }
        }
    }
    else
    {
        errors.push_back("Unsupported AtmosphereModel: " + atmosphere_model);
    }

    if (!parser->HasKey("nSensitiveLayer") && !parser->HasKey("R_sens") && !parser->HasKey("R_sens_1"))
    {
        errors.push_back("Missing sensitive layer definition: set nSensitiveLayer + R_sens_i or set R_sens");
    }

    const std::string generator_model = parser->HasKey("GeneratorModel") ? parser->GetSValue("GeneratorModel") : "";
    if (generator_model == "SphericalSurfaceSampling")
    {
        if (!parser->HasKey("nEvents"))
        {
            errors.push_back("GeneratorModel=SphericalSurfaceSampling requires nEvents");
        }
        for (const auto &key : {"EnergySpectrum", "InitialPosition", "NadirAngle", "AzimuthAngle"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("GeneratorModel=SphericalSurfaceSampling requires key: " + std::string(key));
            }
        }
        validate_sampler("EnergySpectrum", errors);
        validate_sampler("NadirAngle", errors);
        validate_sampler("AzimuthAngle", errors);

        if (!parser->HasKey("InitialPosition") || parser->GetSValue("InitialPosition") != "UniformSphericalSurface")
        {
            errors.push_back("InitialPosition must be UniformSphericalSurface");
        }
        for (const auto &key : {"InitialPosition_mag", "InitialPosition_theta_min", "InitialPosition_theta_max", "InitialPosition_phi_min", "InitialPosition_phi_max"})
        {
            if (!parser->HasKey(key))
            {
                errors.push_back("InitialPosition=UniformSphericalSurface requires key: " + std::string(key));
            }
        }
    }
    else if (generator_model == "GenerateFromTable")
    {
        if (!parser->HasKey("PrimariesDataFile"))
        {
            errors.push_back("GeneratorModel=GenerateFromTable requires PrimariesDataFile");
        }
    }
    else
    {
        errors.push_back("Unsupported GeneratorModel: " + generator_model);
    }

    std::set<std::string> allowed_keys = {
        "ProjectName", "nThreads", "nEvents", "batch_mode", "OutputDir", "Enable_interaction", "Seed",
        "DensityModel", "DensityProfileFile", "r_min", "r_pause", "r_max", "delta_r1", "delta_r2", "delta_theta", "delta_phi",
        "FieldModel", "FieldTableFile", "IGRFDataFile", "Year", "Month", "Day", "n_order_max", "TA16ParamFile", "pdyn", "symhc", "xind", "by_imf", "ps", "vxgse", "vygse", "vzgse",
        "AtmosphereModel", "R_inner", "R_outer", "R_earth", "R_sens", "nSensitiveLayer",
        "DetectingParticleName", "GeneratorModel", "ParticleName", "PrimariesDataFile",
        "EnergySpectrum", "EnergySpectrum_min", "EnergySpectrum_max", "EnergySpectrum_base", "EnergySpectrum_scale", "EnergySpectrum_offset",
        "NadirAngle", "NadirAngle_min", "NadirAngle_max", "NadirAngle_base", "NadirAngle_scale", "NadirAngle_offset",
        "AzimuthAngle", "AzimuthAngle_min", "AzimuthAngle_max", "AzimuthAngle_base", "AzimuthAngle_scale", "AzimuthAngle_offset",
        "InitialPosition", "InitialPosition_mag", "InitialPosition_theta_min", "InitialPosition_theta_max", "InitialPosition_phi_min", "InitialPosition_phi_max"};

    const std::regex r_sens_indexed(R"(^R_sens_[0-9]+$)");
    for (const auto &key : parser->Keys())
    {
        if (allowed_keys.find(key) == allowed_keys.end() && !std::regex_match(key, r_sens_indexed))
        {
            errors.push_back("Unknown config key: " + key);
        }
    }

    if (!errors.empty())
    {
        std::ostringstream message;
        message << "Config validation failed:";
        for (const auto &error : errors)
        {
            message << "\n- " << error;
        }
        throw std::runtime_error(message.str());
    }
}

std::vector<G4double> G4EarthRunManager::resolve_sensitive_layers()
{
    std::vector<G4double> layers;

    if (parser->HasKey("nSensitiveLayer"))
    {
        const int count = parser->GetIValue("nSensitiveLayer");
        if (count < 1)
        {
            throw std::runtime_error("nSensitiveLayer must be >= 1");
        }

        for (int i = 1; i <= count; i++)
        {
            const std::string key = "R_sens_" + std::to_string(i);
            if (parser->HasKey(key))
            {
                layers.push_back(parser->GetDimensionedDValue(key));
            }
            else if (i == 1 && parser->HasKey("R_sens"))
            {
                layers.push_back(parser->GetDimensionedDValue("R_sens"));
            }
            else
            {
                throw std::runtime_error("Missing sensitive-layer key: " + key);
            }
        }
        return layers;
    }

    if (parser->HasKey("R_sens"))
    {
        layers.push_back(parser->GetDimensionedDValue("R_sens"));
        return layers;
    }

    for (int i = 1;; i++)
    {
        const std::string key = "R_sens_" + std::to_string(i);
        if (!parser->HasKey(key))
        {
            break;
        }
        layers.push_back(parser->GetDimensionedDValue(key));
    }

    if (layers.empty())
    {
        throw std::runtime_error("No sensitive layers configured");
    }

    return layers;
}

void G4EarthRunManager::write_run_metadata() const
{
    const std::string metadata_file = output_dir + "/runtime/" + project_name + "_metadata.txt";
    std::ofstream out(metadata_file, std::ofstream::out | std::ofstream::trunc);
    if (!out.is_open())
    {
        G4cerr << "Warning: failed to write run metadata file: " << metadata_file << G4endl;
        return;
    }

    out << "ProjectName=" << project_name << "\n";
    out << "ConfigFile=" << config_file << "\n";
    out << "Threads=" << nThreads << "\n";
    out << "Seed=" << parser->GetIValue("Seed") << "\n";
    out << "DensityModel=" << parser->GetSValue("DensityModel") << "\n";
    out << "FieldModel=" << parser->GetSValue("FieldModel") << "\n";
    out << "AtmosphereModel=" << parser->GetSValue("AtmosphereModel") << "\n";
    out << "GeneratorModel=" << parser->GetSValue("GeneratorModel") << "\n";
    out << "SensitiveLayers=" << sensitive_layers.size() << "\n";
}

void G4EarthRunManager::run()
{
    if (batch_mode)
    {
        timer->start("Batch Mode Execution");
        G4cout << "Starting Batch Execution" << G4endl;

        if (action->get_n_events() == -1)
        {
            G4cout << parser->GetIValue("nEvents") << " events will be generated." << G4endl;
            runManager->BeamOn(parser->GetIValue("nEvents"));
        }
        else
        {
            G4cout << action->get_n_events() << " events will be generated." << G4endl;
            runManager->BeamOn(action->get_n_events());
        }

        G4cout << "Batch Execution Completed" << G4endl;
        timer->stop();
        fileManager->merge_all_output_files();
    }
    else
    {
        G4cerr << "Interactive mode is not supported in this release. Set batch_mode=TRUE." << G4endl;
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
        throw std::runtime_error("Unsupported density model: " + std::string(model_name));
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
        throw std::runtime_error("Unsupported field model: " + std::string(model_name));
    }
}

void G4EarthRunManager::init_atmosphere()
{
    G4String model_name = parser->GetSValue("AtmosphereModel");

    if (model_name == "SphericalEarth")
    {
        atmosphere = new SphericalEarth(
            densityModel,
            sensitive_layers,
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
            sensitive_layers,
            field,
            parser->GetSValue("DetectingParticleName"),
            fileManager);
    }
    else
    {
        throw std::runtime_error("Unsupported atmosphere model: " + std::string(model_name));
    }
}
