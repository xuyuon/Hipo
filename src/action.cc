#include "action.hh"

ActionInitialization::ActionInitialization(Parser *parser)
{
    this->parser= parser;
}

ActionInitialization::~ActionInitialization()
{
    // Destructor implementation
}

void ActionInitialization::Build() const
{
    G4VUserPrimaryGeneratorAction *generator = init_generator();
    SetUserAction(generator); // Set the primary generator action

    RunAction *runAction = new RunAction();
    SetUserAction(runAction);

    EventAction *eventAction = new EventAction(runAction);
    SetUserAction(eventAction); 

    SteppingAction *steppingAction = new SteppingAction(eventAction);
    SetUserAction(steppingAction);
}

G4VUserPrimaryGeneratorAction *ActionInitialization::init_generator() const
{
    G4VUserPrimaryGeneratorAction *generator = nullptr;

    G4String model_name = parser->GetSValue("GeneratorModel");
    std::string project_name = parser->GetSValue("ProjectName");
    std::string output_dir = parser->GetSValue("OutputDir");

    if (model_name == "SphericalSurfaceSampling")
    {
        generator = new SphericalSurfaceSampling(
            parser->GetSValue("ParticleName"),
            get_sampler(parser->GetSValue("EnergySpectrum"), "EnergySpectrum"),
            get_coordinates_sampler(parser->GetSValue("InitialPosition"), "InitialPosition"),
            get_sampler(parser->GetSValue("NadirAngle"), "NadirAngle"),
            get_sampler(parser->GetSValue("AzimuthAngle"), "AzimuthAngle"),
            output_dir + "/primaries/" + project_name + "_primaries" // output directory for sensitive detector
        );
    }
    else if (model_name == "FiniteDifferenceSampling")
    {
        generator = new FiniteDifferenceSampling(
            parser->GetSValue("ParticleName"),
            parser->GetSValue("FDDataFile"),
            output_dir + "/primaries/" + project_name + "_primaries_fd" // output directory for sensitive detector
        );
    }
    else
    {
        G4cerr << "Error: Unsupported generator model: " << model_name << G4endl;
    }
    return generator;
}

void ActionInitialization::BuildForMaster() const
{
    RunAction *runAction = new RunAction();
    SetUserAction(runAction); // Set the run action for the master thread
}

SamplingBase *ActionInitialization::get_sampler(G4String sampler_name, G4String label) const
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

CoordinatesSamplingBase *ActionInitialization::get_coordinates_sampler(G4String sampler_name, G4String label) const
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