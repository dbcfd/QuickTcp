#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

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
    ConnectOverlap(std::shared_ptr<Socket> sckt, const size_t bufferSize);
    virtual ~ConnectOverlap();

    void reset();
    void handleConnection(HANDLE mainIOCP, std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<quicktcp::server::IResponder> responder);
    void transferBufferToStream(const size_t nbBytes);

    bool isReset;
    std::shared_ptr<char> buffer;
    std::shared_ptr<Socket> socket;
    std::shared_ptr<ServerConnection> currentConnection;
    HANDLE iocp;
};

}
}
}
}
