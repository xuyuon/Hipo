#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"

#include "generator.hh"
#include "runAction.hh"
#include "eventAction.hh"
#include "steppingAction.hh"
#include "parser.hh"
#include "sampling.hh"

class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization(Parser *parser, std::string output_dir);
    ~ActionInitialization();

    virtual void Build() const;
    virtual void BuildForMaster() const;

    G4int get_n_events() const;

private:
    // private methods
    GeneratorBase *init_generator() const;
    SamplingBase *get_sampler(G4String sampler_name, G4String label) const;
    CoordinatesSamplingBase *get_coordinates_sampler(G4String sampler_name, G4String label) const;

    // No. of events
    G4int n_events;

    // parser
    Parser *parser;

    // output directory
    std::string output_dir;
};

#endif