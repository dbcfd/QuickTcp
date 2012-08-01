#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Task::Task() : mTaskIsRunning(false), mPerformed(false), mTaskCompletePromise([](Task* task, bool shouldRunTask)->bool {
            task->mTaskIsRunning = true;
            bool ret = false;
            if(shouldRunTask) ret = task->performTask();
            task->mSignal.notify_all();
            return ret;
        } )
{

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

bool Task::perform()
{
    bool performed = mPerformed.exchange(true);
    bool ret = false;
    if(!performed)
    {
        std::future<bool> taskResult = mTaskCompletePromise.get_future();
        mTaskCompletePromise(this, true);
        ret = taskResult.get();
    }
    return ret;
}

bool Task::waitForCompletion()
{
    std::future<bool> future = mTaskCompletePromise.get_future();
    future.wait();
    return future.get();
}

}
}