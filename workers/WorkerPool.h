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
    WorkerPool(const size_t nbWorkers, const bool runInOwnThread = true);
    ~WorkerPool();

    void addWork(Task* workToBeDone);
    void shutdown();
protected:
    void threadEntryPoint();
    void runNextTask(Worker* worker);
private:
    std::vector<Worker*> mWorkers;
    std::queue<Task*> mWaitingTasks;
    std::mutex mMutex;
    std::condition_variable mSignal;
    std::atomic<bool> mRunning;
    std::thread* mThread;
    size_t mNextWorkerToUse;
};

}
}

