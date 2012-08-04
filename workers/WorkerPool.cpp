#include "workers/WorkerPool.h"
#include "workers/Worker.h"
#include "workers/Task.h"

namespace quicktcp {
namespace workers {

WorkerPool::WorkerPool(const size_t nbWorkers) : mShuttingDown(false)
{
    if(nbWorkers <= 0) throw(std::runtime_error("Number of workers must be greater than 0"));
    mWorkers.reserve(nbWorkers);
    Worker::GetTaskFunction gtFunc = std::bind(&WorkerPool::getNextTask, this, std::placeholders::_1);
    Worker::WorkCompleteFunction wcFunc = [this](size_t workerIndex) -> void {
        mWorkerCompleteSignal.notify_all();
    };
    for(size_t i = 0; i < nbWorkers; ++i) {
        mWorkers.push_back(new Worker(gtFunc, wcFunc, i));
    }
}

WorkerPool::~WorkerPool() 
{
    if(!mShuttingDown)
    {
        shutdown();
    }
    std::for_each(mWorkers.begin(), mWorkers.end(), [](Worker* worker) {
        delete worker;
    } );
}

void WorkerPool::shutdownWorker(Worker* worker)
{
    worker->shutdown();
    mTaskSignal.notify_all();
    std::unique_lock<std::mutex> lock(mWorkerCompleteMutex);
    while(worker->isRunning())
    {
        mWorkerCompleteSignal.wait(lock);
    }
}


void WorkerPool::shutdown()
{
    mShuttingDown = true;
    std::for_each(mWorkers.begin(), mWorkers.end(), [this](Worker* worker) {
        shutdownWorker(worker);
    } );
}

std::shared_ptr<Task> WorkerPool::getNextTask(size_t index)
{    
    std::unique_lock<std::mutex> lock(mTaskMutex);
    while(!mShuttingDown && mWaitingTasks.empty())
    {
        mTaskSignal.wait(lock);
    }
    std::shared_ptr<Task> task(nullptr);
    if(!mShuttingDown)
    {
        task = mWaitingTasks.front();
        mWaitingTasks.pop();
    }
    return task;
}

void WorkerPool::addWork(std::shared_ptr<Task> work) 
{
    std::lock_guard<std::mutex> lock(mTaskMutex);
    mWaitingTasks.push(work);
    mTaskSignal.notify_all();
}

}
}