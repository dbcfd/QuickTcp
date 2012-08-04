#pragma once

#include "workers/Platform.h"

#include <atomic>
#include <vector>
#include <future>
#include <queue>
#include <mutex>
#include <memory>

namespace quicktcp {
namespace workers {

class WORKERS_API Worker;
class WORKERS_API Task;

class WORKERS_API WorkerPool {
public:
    WorkerPool(const size_t nbWorkers);
    ~WorkerPool();

    void addWork(std::shared_ptr<Task> workToBeDone);
    void shutdown();
protected:
    std::shared_ptr<Task> getNextTask(size_t workerIndex);
private:
    void shutdownWorker(Worker* worker);

    std::vector<Worker*> mWorkers;
    std::queue<std::shared_ptr<Task>> mWaitingTasks;
    std::mutex mTaskMutex;
    std::condition_variable mTaskSignal;
    std::mutex mWorkerCompleteMutex;
    std::condition_variable mWorkerCompleteSignal;
    std::atomic<bool> mShuttingDown;
};

}
}

