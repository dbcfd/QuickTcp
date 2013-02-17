#pragma once
#include "Workers/Platform.h"
#include "Workers/Task.h"

#include <functional>

namespace markit {
namespace workers {

class WORKERS_API BasicTask : public Task {
public:
    BasicTask(std::function<void(void)> functionToRun);
    virtual ~BasicTask();

protected:
    virtual void PerformSpecific();

private:
    std::function<void(void)> mFunctionToRun;
};

//inline implementations
//------------------------------------------------------------------------------

}
}