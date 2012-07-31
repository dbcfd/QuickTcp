#include "workers/Worker.h"
#include "workers/Task.h"

namespace quicktcp {
namespace workers {

Worker::Worker() 
{
    mThread = new std::thread(std::bind(&Worker::threadEntryPoint, this));
}

Worker::~Worker() 
{
    if(!mShutdown)
    {
        shutdown();
    }
    mThread->join();
    delete mThread;
}

void Worker::threadEntryPoint() 
{
    while(!mShutdown) {
        std::future<std::pair<Task*,WorkCompleteFunction>> futureWork = mPromiseToWork.get_future();
        futureWork.wait();
        std::pair<Task*,WorkCompleteFunction> work = futureWork.get();
        if(nullptr != work.first)
        {
            work.first->perform();
            (work.second)(this);
        }
        mPromiseToWork = std::promise<std::pair<Task*,WorkCompleteFunction>>();
    }
}

void Worker::provideWork(Task* work, WorkCompleteFunction workDone)
{
    mPromiseToWork.set_value(std::make_pair(work, workDone));
}

void Worker::shutdown()
{
    mShutdown = true;
    provideWork(nullptr, [this](Worker*) -> void {});
}

}
}