#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/IOverlap.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <set>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct ResponseOverlap;

//------------------------------------------------------------------------------
struct ReceiveOverlap : public IOverlap {
public:
    ReceiveOverlap(std::shared_ptr<IEventHandler> evHandler, 
        std::shared_ptr<Socket> sckt, 
        std::function<void(void)> onFinish);
    virtual ~ReceiveOverlap();

    virtual void handleIOCompletion(const size_t nbBytes) final;
    virtual void shutdown() final;

private:
    int queueReceive();
    void prepareToReceive();
    void prepareToRespond();

    enum class State {
        Valid,
        ClosingResponses,
        ReadyForFinish
    };

    State mState;
    std::function<void(void)> mOnFinish;
    std::mutex mResponseMutex;
    std::set<std::shared_ptr<ResponseOverlap>> mOutstandingResponses;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
