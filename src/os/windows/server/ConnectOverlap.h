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
class ServerConnection;

//------------------------------------------------------------------------------
struct ConnectOverlap : public IOverlap {
    ConnectOverlap(HANDLE mainIOCP, 
        std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<quicktcp::server::IResponder> responder,
        std::shared_ptr<Socket> sckt, const size_t bufferSize);
    virtual ~ConnectOverlap();

    virtual bool handleIOCompletion(SOCKET sckt, const size_t nbBytes);

    void reset();
    void handleConnection();
    void transferBufferToStream(const size_t nbBytes);
    void prepareToReceive();

    std::atomic_bool isConnected;
    std::shared_ptr<char> buffer;
    std::shared_ptr<Socket> socket;
    std::shared_ptr<IEventHandler> eventHandler;
    std::shared_ptr<quicktcp::server::IResponder> responder;
    DWORD flags;
};

}
}
}
}
