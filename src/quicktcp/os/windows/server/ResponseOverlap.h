#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/IOverlap.h"

#include "async/AsyncResult.h"

#include <functional>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;

//------------------------------------------------------------------------------
struct ResponseOverlap : public IOverlap {
public:
    ResponseOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt);
    virtual ~ResponseOverlap();

    virtual void handleIOCompletion(const size_t nbBytes) final;
    virtual void shutdown() final;

    void setResult(async_cpp::async::AsyncResult&& result);

    inline void setFinishFunction(std::function<void(void)> finishFunction);

private:
    async_cpp::async::AsyncResult mResult;
    std::function<void(void)> mOnFinish;
    bool mShutdown;
};

//Inline Implementations
//------------------------------------------------------------------------------
void ResponseOverlap::setFinishFunction(std::function<void(void)> finishFunction)
{
    mOnFinish = finishFunction;
}

}
}
}
}
