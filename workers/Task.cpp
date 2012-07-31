#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Task::Task() : mFutureTaskComplete(mTaskCompletePromise.get_future()), mTaskCompletedSuccessfully(false)
{

}

Task::~Task()
{

}

void Task::perform()
{
    performTask();
    mTaskCompletedSuccessfully = true;
    mTaskCompletePromise.set_value(true);
}

void Task::waitForCompletion() const
{
    mFutureTaskComplete.wait();
}

void Task::reset()
{
    mTaskCompletePromise.set_value(false);
    mTaskCompletePromise = std::promise<bool>();
    mFutureTaskComplete = mTaskCompletePromise.get_future();
}

}
}