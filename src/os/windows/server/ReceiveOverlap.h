#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

#include <atomic>
#include <functional>
#include <memory>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
class ServerConnection;

//------------------------------------------------------------------------------
struct ReceiveOverlap : public IOverlap {
    ReceiveOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler, 
        std::function<void(void)> onDisconnect);
    virtual ~ReceiveOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);
    virtual std::string isA() { return "Receive"; }
    
    void prepareToReceive();
    void disconnect();

private:
    std::function<void(void)> mOnDisconnect;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
