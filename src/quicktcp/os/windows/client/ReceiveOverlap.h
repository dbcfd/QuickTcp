#pragma once

#include "os/windows/client/Platform.h"
#include "os/windows/client/IOverlap.h"

#include "async/AsyncResult.h"

#include <future>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

class IEventHandler;
class Socket;
class ServerConnection;

//------------------------------------------------------------------------------
class ReceiveOverlap : public IOverlap
{
public:
    ReceiveOverlap(std::shared_ptr<Socket> sckt, 
        const size_t recvBufferSize,
        std::promise<async_cpp::async::AsyncResult>& promise,
        std::shared_ptr<IEventHandler> handler);
    virtual ~ReceiveOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);

    void prepareToReceive();
    void shutdown(const std::string& message);

    std::promise<async_cpp::async::AsyncResult> mPromise;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
