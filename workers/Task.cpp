#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Task::Task() : mTaskIsRunning(false), mPerformed(false), mTaskCompletePromise([](Task* task, bool shouldRunTask)->void {
            task->mTaskIsRunning = true;
            if(shouldRunTask) task->performTask();
            task->mTaskIsRunning = false;
            task->mSignal.notify_all();
        } ), mTaskIsComplete(false)
{
    mTaskComplete = mTaskCompletePromise.get_future();
}

Task::~Task()
{
    bool performed = mPerformed.exchange(true);
    if(!performed)
    {
        mTaskCompletePromise(this, false);
        
    }
    {
        std::unique_lock<std::mutex> lock(mSignalMutex);
        while(mTaskIsRunning)
        {
            mSignal.wait(lock);
        }
    }
}

void Task::perform()
{
    bool performed = mPerformed.exchange(true);
    if(!performed)
    {
        mTaskCompletePromise(this, true);
    }
    mTaskIsComplete = true;
}

void Task::waitForCompletion()
{
    mTaskComplete.wait();
}

}
}