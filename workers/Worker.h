#pragma once

#include "workers/Platform.h"

#include <future>
#include <atomic>
#include <functional>

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
    void threadEntryPoint();

    std::promise< std::pair<Task*, WorkCompleteFunction> > mPromiseToWork;
    std::atomic<bool> mShutdown;
    std::thread* mThread;
};

}
}