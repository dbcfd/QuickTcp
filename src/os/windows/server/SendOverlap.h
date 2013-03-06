#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

#include "async/AsyncResult.h"

#include <atomic>
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
    SendOverlap(SOCKET sckt, std::shared_ptr<utilities::ByteStream> str);
    ~SendOverlap();

    virtual bool handleIOCompletion(SOCKET sckt, const size_t nbBytes);

    void completeSend(const size_t nbBytes);

    std::atomic_bool sendComplete;
    std::promise<async_cpp::async::AsyncResult> promise;
    SOCKET socket;
};

}
}
}
}
