#pragma once

#include "os/windows/Server/Platform.h"

#include <future>
#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace os {
namespace windows {
namespace server {

struct ConnectOverlap;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class IEventHandler {
public:
    virtual void queueAccept(ConnectOverlap& overlap) = 0;
    virtual void handleResponse(SOCKET sckt, std::shared_ptr<utilities::ByteStream> stream) = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
