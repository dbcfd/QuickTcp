#pragma once

#include "quicktcp/os/windows/Server/Platform.h"

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
struct IOverlap;
struct ResponseOverlap;
class Socket;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class IEventHandler {
public:
    virtual ~IEventHandler();

    virtual void postCompletion(IOverlap* overlap) = 0;
    virtual void queueAccept(ConnectOverlap* overlap) = 0;
    virtual bool authenticateConnection(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void createResponse(std::shared_ptr<utilities::ByteStream> stream, std::shared_ptr<ResponseOverlap> overlap) = 0;
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
