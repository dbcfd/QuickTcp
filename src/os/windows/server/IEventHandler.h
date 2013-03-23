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
struct ReceiveOverlap;
struct ResponseOverlap;
class Socket;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class IEventHandler {
public:
    virtual void queueAccept(ConnectOverlap& overlap) = 0;
    virtual void authenticateConnection(std::shared_ptr<utilities::ByteStream> stream, ReceiveOverlap* overlap) = 0;
    virtual void createResponse(std::shared_ptr<utilities::ByteStream> stream, ResponseOverlap* overlap) = 0;
    virtual void sendResponse(std::shared_ptr<Socket> socket, std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void reportError(const std::string& error) = 0;
    virtual void connectionClosed() = 0;
    virtual size_t connectBufferSize() const = 0;
    virtual size_t receiveBufferSize() const = 0;
    virtual size_t responseBufferSize() const = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
