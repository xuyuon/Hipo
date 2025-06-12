#include "action.hh"

ActionInitialization::ActionInitialization()
{
    // Constructor implementation
}

ActionInitialization::~ActionInitialization()
{
    // Destructor implementation
}

void ActionInitialization::Build() const
{
    PrimaryGenerator *generator = new PrimaryGenerator();
    SetUserAction(generator); // Set the primary generator action

    RunAction *runAction = new RunAction();
    SetUserAction(runAction);

    EventAction *eventAction = new EventAction(runAction);
    SetUserAction(eventAction); 

    SteppingAction *steppingAction = new SteppingAction(eventAction);
    SetUserAction(steppingAction);
}


void ActionInitialization::BuildForMaster() const
{
    RunAction *runAction = new RunAction();
    SetUserAction(runAction); // Set the run action for the master thread
}