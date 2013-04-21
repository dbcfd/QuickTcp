#pragma once

#include "quicktcp/os/windows/client/Platform.h"
#include "quicktcp/os/windows/client/IOverlap.h"

#include "async/Async.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

class Socket;

//------------------------------------------------------------------------------
class IEventHandler
{
public:
    virtual ~IEventHandler();
    virtual void createReceiveOverlap(std::shared_ptr<Socket> socket, std::promise<async_cpp::async::AsyncResult>& promise) = 0;
    virtual async_cpp::async::AsyncFuture processStream(std::shared_ptr<utilities::ByteStream> stream) = 0;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
