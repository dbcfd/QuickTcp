#pragma once

#include "os/windows/client/Platform.h"
#include "os/windows/client/IOverlap.h"

#include<future>

namespace async_cpp {
namespace async {
class AsyncResult;
}
}

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

class Socket;

//------------------------------------------------------------------------------
class IEventHandler
{
public:
    virtual void createReceiveOverlap(std::shared_ptr<Socket> socket, std::promise<async_cpp::async::AsyncResult>& promise) = 0;
    virtual async_cpp::async::AsyncResult processStream(std::shared_ptr<utilities::ByteStream> stream) = 0;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
