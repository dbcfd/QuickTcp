#pragma once
#include "Workers/Platform.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <thread>

namespace markit {
namespace workers {

class Task;

class WORKERS_API Worker {
public:
    Worker(std::function<void (Worker*)> taskCompleteFunction);
    virtual ~Worker();

    void RunTask(std::shared_ptr<Task> task);
    void Shutdown();

    inline void WaitUntilReady();
    inline const bool IsRunning();
private:
    void Run();

    std::unique_ptr<std::thread> mThread;
    std::promise<bool> mReadyForWorkPromise;
    std::future<bool> mReadyForWorkFuture;
    std::mutex mTaskMutex;
    std::shared_ptr<Task> mTaskToRun;
    std::condition_variable mTaskSignal;
    std::function<void (Worker*)> mTaskCompleteFunction;
    std::atomic<bool> mRunning;
};

//inline implementations
//------------------------------------------------------------------------------
void Worker::WaitUntilReady()
{
     mReadyForWorkFuture.wait();
}

//------------------------------------------------------------------------------
const bool Worker::IsRunning()
{
    return mRunning;
}

}
}