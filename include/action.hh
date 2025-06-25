#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"

#include "generator.hh"
#include "runAction.hh"
#include "eventAction.hh"
#include "steppingAction.hh"


class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization(G4VUserPrimaryGeneratorAction* generator);
    ~ActionInitialization();

    virtual void Build() const;
    virtual void BuildForMaster() const;

private:
    G4VUserPrimaryGeneratorAction* generator;
};


#endif