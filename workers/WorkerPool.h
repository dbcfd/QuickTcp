#pragma once

#include "workers/Platform.h"

#include <atomic>
#include <vector>
#include <future>
#include <queue>
#include <mutex>

namespace quicktcp {
namespace workers {

class WORKERS_API Worker;
class WORKERS_API Task;

class WORKERS_API WorkerPool {
public:
    WorkerPool(const size_t nbWorkers);
    ~WorkerPool();

    void addWork(Task* workToBeDone);
    void shutdown();
protected:
    void threadEntryPoint();
private:
    std::packaged_task<bool()> generateWorkerReady();
    std::packaged_task<bool()> generateTaskReady();
    std::queue<Worker*> mWorkers;
    std::queue<Task*> mWaitingTasks;
    std::packaged_task<bool()>* mTaskReady;
    std::packaged_task<bool()>* mWorkerReady;
    std::mutex mProcessingMutex;
    bool mShutdown;
    std::thread* mThread;
    std::atomic<int> mWorkersRunning;
};

}
}

