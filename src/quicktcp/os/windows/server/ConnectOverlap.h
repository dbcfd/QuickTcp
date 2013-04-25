#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/IOverlap.h"

#include <atomic>
#include <memory>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct ReceiveOverlap;

//------------------------------------------------------------------------------
struct ConnectOverlap : public IOverlap {
public:
    ConnectOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt);
    virtual ~ConnectOverlap();

    virtual void shutdown() final;
    void prepareForClientConnection(SOCKET serverSocket);

    inline bool isShutdown();

private:
    virtual void handleIOCompletion(const size_t nbBytes) final;

    void disconnect();
    void handleConnection();

    bool mShuttingDown;
    bool mPendingDisconnect;
    std::shared_ptr<ReceiveOverlap> mReceiver;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool ConnectOverlap::isShutdown()
{
    return mShuttingDown && !mReceiver;
}

}
}
}
}
