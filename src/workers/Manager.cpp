#include "Workers/Manager.h"
#include "Workers/Worker.h"
#include "Workers/Task.h"

#include <functional>

namespace markit {
namespace workers {

//------------------------------------------------------------------------------
Manager::Manager(const size_t nbWorkers) : mShutdown(false), mNbWorkers(nbWorkers)
{
    auto workerDoneFunction = [this](Worker* worker) -> void {
        this->WorkerDone(worker);
    };

    std::vector<Worker*> workers;
    workers.reserve(nbWorkers);
    for(size_t workerIdx = 0; workerIdx < nbWorkers; ++workerIdx)
    {
        workers.emplace_back(new Worker(workerDoneFunction));
    }

    for(Worker* worker : workers)
    {
        worker->WaitUntilReady();
        mWorkers.push(std::move(worker));
    }
}

//------------------------------------------------------------------------------
Manager::~Manager()
{
    Shutdown();
}

//------------------------------------------------------------------------------
void Manager::Shutdown()
{
    bool wasShutdown = !(mShutdown.exchange(true));

    if(wasShutdown)
    {
        std::queue< std::shared_ptr<Task> > tasks;
        {
            std::unique_lock<std::mutex> lock(mMutex);

            //clear out tasks
            {
                std::queue< std::shared_ptr<Task> > empty;
                std::swap(empty, mTasks);
            }

            //wait for all running tasks to finish
            mWorkerFinishedSignal.wait(lock, [this]()->bool {
                return mWorkers.size() == mNbWorkers;
            } );
        }

        while(!tasks.empty())
        {
            Run(tasks.front());
            tasks.pop();
        }

        while(!mWorkers.empty())
        {
            Worker* worker = mWorkers.front();
            mWorkers.pop();
            worker->Shutdown();
            delete worker;
        }
    }
}

//------------------------------------------------------------------------------
void Manager::WorkerDone(Worker* worker)
{
    //grab the next task if available, otherwise add our worker to a wait list
    if(!IsShutdown())
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mMutex);

            if(mTasks.empty())
            {
                mWorkers.push(worker);
            }
            else
            {
                //task available, run it
                task.swap(mTasks.front());
                mTasks.pop();
            }
        }
        if(nullptr != task)
        {
            worker->RunTask(task);
        }
        else
        {
            mWorkerFinishedSignal.notify_one();
        }
    }
}

//------------------------------------------------------------------------------
void Manager::WaitForTasksToComplete()
{
    std::unique_lock<std::mutex> lock(mMutex);

    mWorkerFinishedSignal.wait(lock, [this]()->bool {
        return mTasks.empty();
    } );
}

//------------------------------------------------------------------------------
void Manager::Run(std::shared_ptr<Task> task)
{
    //we want to run this task in a worker if one is available, else, add it to a queue
    if(!IsShutdown())
    {
        Worker* worker = nullptr;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if(mWorkers.empty())
            {
                //no workers available, queue the task
                mTasks.push(task);
            }
            else
            {
                //worker available, grab it, and run task
                worker = mWorkers.front();
                mWorkers.pop();
            }
        }
        if(nullptr != worker)
        {
            worker->RunTask(task);
        }
    }
    else if(task != nullptr)
    {
        task->FailToPerform();
    }
}

}
}
