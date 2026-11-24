#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"

#include "eventAction.hh"


class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(EventAction* eventAction, bool record_tracks=false);
    ~SteppingAction();

    virtual void UserSteppingAction(const G4Step* step);
private:
    G4double E_min;
};

#endif
