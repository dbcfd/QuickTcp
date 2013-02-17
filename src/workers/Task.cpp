#include "Workers/Task.h"

#include <iostream>

namespace markit {
namespace workers {

//------------------------------------------------------------------------------
Task::Task()
{

}

//------------------------------------------------------------------------------
Task::~Task()
{
    try {
        //attempt to fail the task, which sets the future to false
        FailToPerform();
    }
    catch(std::future_error&)
    {
        //task was successfully performed
    }
}

//------------------------------------------------------------------------------
void Task::FailToPerform()
{
    mTaskCompletePromise.set_value(false);
}

//------------------------------------------------------------------------------
void Task::Perform(std::function<void(void)> completeFunction)
{
    bool successful = false;
    try 
    {
        PerformSpecific();
        successful = true;
    }
    catch(std::runtime_error& err)
    {
        std::cerr << "Task failure: " << err.what() << std::endl;
    }

    completeFunction();
    mTaskCompletePromise.set_value(successful);
}

}
}
