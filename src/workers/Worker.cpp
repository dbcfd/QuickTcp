#include "Workers/Worker.h"
#include "Workers/Task.h"

namespace markit {
namespace workers {

//------------------------------------------------------------------------------
Worker::Worker(std::function<void (Worker*)> taskCompleteFunction) : mRunning(true), mTaskCompleteFunction(taskCompleteFunction)
{
    mReadyForWorkFuture = mReadyForWorkPromise.get_future();
    mThread = std::unique_ptr<std::thread>(new std::thread(&Worker::Run, this));
}

//------------------------------------------------------------------------------
Worker::~Worker()
{
    Shutdown();
}

//------------------------------------------------------------------------------
void Worker::Shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    
    if(wasRunning)
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);
            task.swap(mTaskToRun);
        }
        mTaskSignal.notify_all();
        mThread->join();
        if(nullptr != task)
        {
            mTaskCompleteFunction(this);
            task->FailToPerform();
        }
    }
}

//------------------------------------------------------------------------------
void Worker::RunTask(std::shared_ptr<Task> task)
{
    if(IsRunning())
    {
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);
            mTaskToRun = task;
        }

        mTaskSignal.notify_all();
    }
    else if(nullptr != task)
    {
        task->FailToPerform();
    }
}

//------------------------------------------------------------------------------
void Worker::Run()
{
    mReadyForWorkPromise.set_value(true);

    while(IsRunning())
    {
        std::shared_ptr<Task> taskToRun;
        {
            std::unique_lock<std::mutex> lock(mTaskMutex);

            mTaskSignal.wait(lock, [this]()->bool {
                return (nullptr != mTaskToRun) || !IsRunning();
            } );

            taskToRun.swap(mTaskToRun);
        }

        if(nullptr != taskToRun)
        {
            taskToRun->Perform([this]()->void {
                mTaskCompleteFunction(this);
            } );
        }
    }
}

}
}
