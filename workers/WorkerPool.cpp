#include "workers/WorkerPool.h"
#include "workers/Worker.h"

namespace quicktcp {
namespace workers {

WorkerPool::WorkerPool(const size_t nbWorkers) 
{
    mTaskReady = new std::packaged_task<bool(void)>(generateTaskReady());
    mWorkerReady = new std::packaged_task<bool(void)>(generateWorkerReady());
    mThread = new std::thread(std::bind(&WorkerPool::threadEntryPoint, this));
    if(nbWorkers <= 0) throw(std::runtime_error("Number of workers must be greater than 0"));
    for(size_t i = 0; i < nbWorkers; ++i) {
        mWorkers.push(new Worker());
    }
    std::move(mWorkerReady);
}

std::packaged_task<bool()> WorkerPool::generateTaskReady()
{
    return std::packaged_task<bool()>([this]()->bool {
        mTaskReady->swap(generateTaskReady());
        return true;
    } );
}

std::packaged_task<bool()> WorkerPool::generateWorkerReady()
{
    return std::packaged_task<bool()>([this]()->bool {
        mWorkerReady->swap(generateWorkerReady());
        return true;
    } );
}

WorkerPool::~WorkerPool() 
{
    shutdown();
    while(0 != mWorkersRunning)
    {
        while(!mWorkers.empty())
        {
            Worker* worker = mWorkers.front();
            mWorkers.pop();
            delete worker;
        }
        std::future<bool> futureWorker = mWorkerReady->get_future();
        futureWorker.wait();
    }   
    delete mWorkerReady;
    delete mTaskReady;
}

void WorkerPool::shutdown()
{
    mShutdown = true;
    std::move(*mWorkerReady);
    std::move(*mTaskReady);
}

void WorkerPool::threadEntryPoint()
{
    Task* task;
    Worker* worker;
    while(!mShutdown)
    {
        //wait for a worker
        std::future<bool> futureWorker = mWorkerReady->get_future();
        futureWorker.wait();
        std::future<bool> futureTask = mTaskReady->get_future();
        futureTask.wait();

        bool canProcess = true;
        while(!mShutdown && canProcess)
        {
            //grab a task and worker
            {
                std::lock_guard<std::mutex> lock(mProcessingMutex);
                task = mWaitingTasks.front();
                mWaitingTasks.pop();
                worker = mWorkers.front();
                mWorkers.pop();
                canProcess = !(mWaitingTasks.empty() && mWorkers.empty());
            }
            ++mWorkersRunning;
            worker->provideWork(task, [this](Worker* worker) -> void {
                std::lock_guard<std::mutex> lock(mProcessingMutex);
                mWorkers.push(worker);
                --mWorkersRunning;
                std::move(*mWorkerReady);
            } );
        }
    }
}

void WorkerPool::addWork(Task* work) 
{
    if(nullptr != work)
    {
        //add task to the  queue
        {
            std::lock_guard<std::mutex> lock(mProcessingMutex);
            mWaitingTasks.push(work);
        }
        std::move(*mTaskReady);
    }
}

}
}