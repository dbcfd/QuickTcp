#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

#include "async/AsyncResult.h"

#include <future>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
class Socket;
class ServerConnection;

//------------------------------------------------------------------------------
class SendOverlap : public IOverlap
{
public:
    SendOverlap(std::shared_ptr<utilities::ByteStream> str);
    ~SendOverlap();

    std::promise<async_cpp::async::AsyncResult> promise;
};

}
}
}
}
