#include "workers/WorkerPool.h"
#include "workers/Worker.h"

namespace quicktcp {
namespace workers {

WorkerPool::WorkerPool(const size_t nbWorkers, const bool runInOwnThread) : mRunning(false)
{
    if(nbWorkers <= 0) throw(std::runtime_error("Number of workers must be greater than 0"));
    mWorkers.reserve(nbWorkers);
    for(size_t i = 0; i < nbWorkers; ++i) {
        mWorkers.push_back(new Worker());
    }
    mNextWorkerToUse = 0;
    if(runInOwnThread)
    {
        mThread = new std::thread(std::bind(&WorkerPool::threadEntryPoint, this));
        std::unique_lock<std::mutex> lock(mMutex);
        while(!mRunning)
        {
            mSignal.wait(lock);
        }
    }
}

WorkerPool::~WorkerPool() 
{
    shutdown();
    std::for_each(mWorkers.begin(), mWorkers.end(), [](Worker* worker) {
        delete worker;
    } );
}

void WorkerPool::shutdown()
{
    mRunning = false;
    mSignal.notify_all();
}

void WorkerPool::threadEntryPoint()
{
    while(mRunning)
    {
        Worker* worker = mWorkers[mNextWorkerToUse];
        ++mNextWorkerToUse;
        runNextTask(worker);
    }
}

void WorkerPool::runNextTask(Worker* worker)
{
    std::unique_lock<std::mutex> lock(mMutex);
    while(mRunning && mWaitingTasks.empty())
    {
        mSignal.wait(lock);
    }
    if(mRunning)
    {
        Task* task = mWaitingTasks.front();
        mWaitingTasks.pop();
        worker->provideWork(task, [this](Worker* worker) -> void {
            runNextTask(worker);
        } );
    }
}

void WorkerPool::addWork(Task* work) 
{
    std::lock_guard<std::mutex> lock(mMutex);
    mWaitingTasks.push(work);
    mSignal.notify_all();
}

}
}