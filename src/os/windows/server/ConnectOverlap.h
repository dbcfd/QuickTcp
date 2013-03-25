#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

#include <atomic>
#include <memory>

namespace quicktcp {

namespace server {
class IResponder;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct ReceiveOverlap;

//------------------------------------------------------------------------------
struct ConnectOverlap : public IOverlap {
    ConnectOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler,
        HANDLE mainIOCP);
    virtual ~ConnectOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);
    virtual std::string isA() { return "Connect"; }

    void reset();
    void handleConnection();
    void waitForDisconnect();

private:
    ReceiveOverlap* mReceiver;
    bool mPendingDisconnect;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
