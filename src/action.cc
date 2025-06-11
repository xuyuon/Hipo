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

    MyRunAction *runAction = new MyRunAction();
    SetUserAction(runAction);
}