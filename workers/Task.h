#pragma once

#include "workers/Platform.h"

#include <future>

namespace quicktcp {
namespace workers {

class WORKERS_API Task {
public :
   Task();
   virtual ~Task();

   void perform();
   
   inline bool completedSuccessfully() const;

   void waitForCompletion() const;
   void reset();
protected:
	virtual void performTask() = 0;
private:
	std::promise<bool> mTaskCompletePromise;
    std::future<bool> mFutureTaskComplete;
    bool mTaskCompletedSuccessfully;
};

//Inline methods
bool Task::completedSuccessfully() const
{
    return mTaskCompletedSuccessfully;
}

}
}