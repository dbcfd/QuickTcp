#include "Workers/BasicTask.h"

namespace markit {
namespace workers {

//------------------------------------------------------------------------------
BasicTask::BasicTask(std::function<void(void)> functionToRun) : mFunctionToRun(functionToRun)
{

}

//------------------------------------------------------------------------------
BasicTask::~BasicTask()
{

}

//------------------------------------------------------------------------------
void BasicTask::PerformSpecific()
{
    mFunctionToRun();
}

}
}
