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
    typedef std::function<Task*(size_t)> GetTaskFunction;
    typedef std::function<void(size_t)> WorkCompleteFunction;
    Worker(GetTaskFunction gtFunc, WorkCompleteFunction rfFunc, size_t index);
    ~Worker();

    void shutdown();

    inline bool isRunning() const;

private:
    void threadEntryPoint(GetTaskFunction func, WorkCompleteFunction rfFunc, size_t index);

    std::condition_variable mStartupSignal;
    std::mutex mStartupMutex;
    std::atomic<bool> mRunning;
    std::thread* mThread;
    GetTaskFunction mGetTask;
};

//inline implementations
bool Worker::isRunning() const
{
    return mRunning;
}

}
}