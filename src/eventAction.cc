#include "eventAction.hh"


EventAction::EventAction(RunAction* runAction)
{

}


EventAction::~EventAction()
{

}


void EventAction::BeginOfEventAction(const G4Event*)
{
    // Initialization code for the beginning of an event
    // For example, reset energy deposit or other variables
}

void EventAction::EndOfEventAction(const G4Event*)
{

}