#pragma once

#include "workers/Platform.h"

#include <atomic>
#include <future>
#include <condition_variable>

namespace quicktcp {
namespace workers {

class WORKERS_API Task {
public :
   Task();
   virtual ~Task();

   /**
    * Starts the task operation.
    */
   void perform();

   void waitForCompletion();

   inline bool hasBeenPerformed() const;
   inline bool isComplete() const;

protected:
	virtual void performTask() = 0;
    void notifyTaskComplete();
private:
	std::packaged_task<void(Task*,bool)> mTaskCompletePromise;
    std::future<void> mTaskComplete;
    std::condition_variable mSignal;
    std::mutex mSignalMutex;
    bool mTaskIsRunning;
    bool mTaskIsComplete;
    std::atomic<bool> mPerformed;
};

//inline
bool Task::hasBeenPerformed() const
{
    return mPerformed;
}

bool Task::isComplete() const
{
    return mTaskIsComplete;
}

}
}