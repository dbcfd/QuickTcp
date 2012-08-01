#pragma once

#include "workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <mutex>

namespace quicktcp {
namespace workers {

class WORKERS_API Task;

class WORKERS_API Worker {
public :
    typedef std::function<void(Worker*)> WorkCompleteFunction;
    Worker();
    ~Worker();

    void provideWork(Task* work, WorkCompleteFunction workDone);
    void shutdown();

private:
    std::packaged_task<bool(Task*,WorkCompleteFunction)> getWorkPromise();
    void threadEntryPoint();

    std::packaged_task<bool(Task*,WorkCompleteFunction)> mWorkPromise;
    std::atomic<bool> mRunning;
    std::atomic<bool> mWorkReady;
    std::condition_variable mWorkSignal;
    std::mutex mWorkMutex;
    std::thread* mThread;
    Task* mTask;
    WorkCompleteFunction mWorkDone;
};

}
}