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
   bool perform();

   bool waitForCompletion();

protected:
	virtual bool performTask() = 0;
    void notifyTaskComplete();
private:
	std::packaged_task<bool(Task*,bool)> mTaskCompletePromise;
    std::condition_variable mSignal;
    std::mutex mSignalMutex;
    bool mTaskIsRunning;
    std::atomic<bool> mPerformed;
};

}
}