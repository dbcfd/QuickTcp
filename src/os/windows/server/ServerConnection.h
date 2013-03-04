#pragma once

#include "os/windows/Server/Platform.h"
#include "os/windows/server/Winsock2.h"

#include "server/IServerConnection.h"

#include <atomic>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct ConnectOverlap;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class WINDOWSSERVER_API ServerConnection: public quicktcp::server::IServerConnection
{
public:
	/**
	 * Utilizes the socket created through an asynchronous accept (AcceptEx), to maintain a connection
	 * between a server and a client
	 */
    ServerConnection(ConnectOverlap& overlap, 
        std::shared_ptr<IEventHandler> evHandler,
        std::shared_ptr<quicktcp::server::IResponder> responder);
	virtual ~ServerConnection();

    virtual void disconnect();

    void prepareToReceive();
    void processResponse(std::shared_ptr<utilities::ByteStream> stream);

private:
    ServerConnection(const ServerConnection& other);

    ConnectOverlap& mOverlap;
    std::shared_ptr<IEventHandler> mEventHandler;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
