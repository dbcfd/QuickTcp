#include "workers/Worker.h"
#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Worker::Worker() : mRunning(false), mWorkReady(false), mWorkPromise(getWorkPromise())
{
    std::unique_lock<std::mutex> lock(mWorkMutex);
    mThread = new std::thread(std::bind(&Worker::threadEntryPoint, this));
    while(!mRunning)
    {
        mWorkSignal.wait(lock);
    }
}

std::packaged_task<bool(Task*, Worker::WorkCompleteFunction)> Worker::getWorkPromise()
{
    return std::packaged_task<bool(Task*,WorkCompleteFunction)>([this](Task* task, WorkCompleteFunction func) -> bool {
        std::future<bool> workDone = mWorkPromise.get_future();
        workDone.wait();
        bool ret = false;
        if(nullptr != task) ret = task->perform();
        mWorkPromise.swap(getWorkPromise());
        func(this);
        return ret;
    } );
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

void Worker::threadEntryPoint() 
{
    mRunning = true;
    mWorkSignal.notify_all();
    while(mRunning)
    {
        std::unique_lock<std::mutex> lock(mWorkMutex);
        while(!mWorkReady) 
        {
            mWorkSignal.wait(lock);
        }
        if(mRunning) mWorkPromise(mTask, mWorkDone);
    }
}

void Worker::provideWork(Task* work, WorkCompleteFunction workDone)
{
    if(mRunning)
    {
        mTask = work;
        mWorkDone = workDone;
        mWorkReady = true;
        mWorkSignal.notify_all();
    }
}

void Worker::shutdown()
{
    mRunning = false;
    mWorkPromise(nullptr, [](Worker*) -> void {} );
}

}
}