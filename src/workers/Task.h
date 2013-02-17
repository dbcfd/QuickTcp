#pragma once
#include "Workers/Platform.h"

#include <functional>
#include <future>

namespace markit {
namespace workers {

class WORKERS_API Task {
public:
    Task();
    virtual ~Task();

    inline std::future<bool> GetCompletionFuture();

    void Perform(std::function<void(void)> priorToCompleteFunction);
    void FailToPerform();

protected:
    virtual void PerformSpecific() = 0;

private:
    std::promise<bool> mTaskCompletePromise;
};

//inline implementations
//------------------------------------------------------------------------------
std::future<bool> Task::GetCompletionFuture()
{
    return mTaskCompletePromise.get_future();
}

}
}