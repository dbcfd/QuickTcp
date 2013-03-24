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
class SendOverlap : public IOverlap
{
public:
    SendOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<utilities::ByteStream> stream,
        std::shared_ptr<IEventHandler> handler);
    ~SendOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);

    void completeSend();
    void shutdown(const std::string& message);

    std::promise<async_cpp::async::AsyncResult> mPromise;

};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
