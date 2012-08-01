#include "workers/Worker.h"
#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Worker::Worker(GetTaskFunction gtFunc, WorkCompleteFunction wcFunc, size_t index) : mRunning(false)
{
    mThread = new std::thread(std::bind(&Worker::threadEntryPoint, this, gtFunc, wcFunc, index));
    std::unique_lock<std::mutex> lock(mStartupMutex);
    while(!mRunning)
    {
        mStartupSignal.wait(lock);
    }
}

Worker::~Worker() 
{
    if(mRunning)
    {
        shutdown();
    }
    mThread->join();
    delete mThread;
}

void Worker::threadEntryPoint(GetTaskFunction gtFunc, WorkCompleteFunction wcFunc, size_t index) 
{
    mRunning = true;
    mStartupSignal.notify_all();
    while(mRunning)
    {
        Task* task = gtFunc(index);
        if(nullptr != task) task->perform();
        wcFunc(index);
    }
}

void Worker::shutdown()
{
    mRunning = false;
}

}
}