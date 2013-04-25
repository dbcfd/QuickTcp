#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/IOverlap.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
class Socket;

//------------------------------------------------------------------------------
struct SendOverlap : public IOverlap {
public:
    SendOverlap(std::shared_ptr<IEventHandler> evHandler, 
        std::shared_ptr<Socket> sckt,
        std::shared_ptr<utilities::ByteStream> stream);
    ~SendOverlap();

    virtual void handleIOCompletion(const size_t nbBytes) final;
    virtual void shutdown() final;
    int queueSend();

private:
    void completeSend();
    size_t mExpectedSize;
    size_t mTotalBytes;
    bool mShutdown;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
