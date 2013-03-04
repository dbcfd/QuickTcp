#pragma once

#include "os/windows/Server/Platform.h"

#include <future>

namespace async_cpp {
namespace async {
class AsyncResult;
}
}

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class ServerConnection;
/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class IEventHandler {
public:
    virtual void disconnected(ServerConnection& cnct) = 0;
    virtual void handleResponse(std::future<async_cpp::async::AsyncResult>& response) = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
