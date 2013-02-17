#pragma once
#include "Workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace markit {
namespace workers {

class Task;
class Worker;

class WORKERS_API Manager {
public:
    Manager(const size_t nbWorkers);
    ~Manager();

    void Run(std::shared_ptr<Task> task);
    void Shutdown();
    void WaitForTasksToComplete();

    inline const bool IsShutdown();
protected:
    void WorkerDone(Worker* worker);
    void Run();

    size_t mNbWorkers;
    std::queue< Worker* > mWorkers;

    std::mutex mMutex;
    std::condition_variable mWorkerFinishedSignal;

    std::queue< std::shared_ptr<Task> > mTasks;

    std::atomic<bool> mShutdown;
};

//inline implementations
//------------------------------------------------------------------------------
const bool Manager::IsShutdown()
{
    return mShutdown;
}

}
}