#include "action.hh"

ActionInitialization::ActionInitialization(Parser *parser, OutputManager *outputManager)
{
    this->parser = parser;
    this->outputManager = outputManager;

    this->n_events = -1; // default value, if generator does not specify it
    if (parser->GetSValue("GeneratorModel") == "GenerateFromTable")
    {
        // generate a dummy generator to get the number of events
        GeneratorBase *generator = new GenerateFromTable(
            parser->GetSValue("ParticleName"),
            parser->GetSValue("PrimariesDataFile"),
            outputManager
        );
        this->n_events = generator->get_n_events();
        delete generator; // clean up the dummy generator
    }
}

ActionInitialization::~ActionInitialization()
{
    // Destructor implementation
}

void ActionInitialization::Build() const
{
    GeneratorBase *generator = init_generator();
    SetUserAction(generator); // Set the primary generator action

    RunAction *runAction = new RunAction();
    SetUserAction(runAction);

    EventAction *eventAction = new EventAction(runAction);
    SetUserAction(eventAction);

    SteppingAction *steppingAction = new SteppingAction(eventAction);
    SetUserAction(steppingAction);
}

GeneratorBase *ActionInitialization::init_generator() const
{
    GeneratorBase *generator = nullptr;

    G4String model_name = parser->GetSValue("GeneratorModel");

    if (model_name == "SphericalSurfaceSampling")
    {
        generator = new SphericalSurfaceSampling(
            parser->GetSValue("ParticleName"),
            get_sampler(parser->GetSValue("EnergySpectrum"), "EnergySpectrum"),
            get_coordinates_sampler(parser->GetSValue("InitialPosition"), "InitialPosition"),
            get_sampler(parser->GetSValue("NadirAngle"), "NadirAngle"),
            get_sampler(parser->GetSValue("AzimuthAngle"), "AzimuthAngle"),
            outputManager
        );
    }
    else if (model_name == "GenerateFromTable")
    {
        generator = new GenerateFromTable(
            parser->GetSValue("ParticleName"),
            parser->GetSValue("PrimariesDataFile"),
            outputManager
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
    else if (sampler_name == "Lambertian")
    {
        return new Lambertian();
    }
    else if (sampler_name == "UpwardLambertian")
    {
        return new UpwardLambertian();
    }
    else if (sampler_name == "Beta")
    {
        return new Beta(parser->GetDValue(label + "_alpha"), parser->GetDValue(label + "_beta"), parser->GetDimensionedDValue(label + "_min"), parser->GetDimensionedDValue(label + "_max"));
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

G4int ActionInitialization::get_n_events() const
{
    return n_events;
}

CoordinatesSamplingBase *ActionInitialization::get_coordinates_sampler(G4String sampler_name, G4String label) const
{
    if (sampler_name == "UniformSphericalSurface")
    {
        return new UniformSphericalSurface(
            parser->GetDimensionedDValue(label + "_mag"),
            parser->GetDimensionedDValue(label + "_theta_min"),
            parser->GetDimensionedDValue(label + "_theta_max"),
            parser->GetDimensionedDValue(label + "_phi_min"),
            parser->GetDimensionedDValue(label + "_phi_max")
            );
    }
    else
    {
        G4cerr << "Error: Unsupported coordinates sampler: " << sampler_name << G4endl;
        return nullptr;
    }
}
