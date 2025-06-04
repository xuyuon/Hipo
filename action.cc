#include "action.hh"

MyActionInitialization::MyActionInitialization()
{
    // Constructor implementation
}

MyActionInitialization::~MyActionInitialization()
{
    // Destructor implementation
}

void MyActionInitialization::Build() const
{
    MyPrimaryGenerator *generator = new MyPrimaryGenerator();
    SetUserAction(generator); // Set the primary generator action

    MyRunAction *runAction = new MyRunAction();
    SetUserAction(runAction);
}