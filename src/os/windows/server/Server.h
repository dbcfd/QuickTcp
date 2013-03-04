#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/Winsock2.h"

#include "server/IServer.h"
#include "server/IServerConnection.h"

#include <map>

namespace async_cpp {
namespace workers {
class IManager;
}
}

namespace quicktcp {
namespace server {
class IResponder;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct ConnectOverlap;
class Socket;
class ServerConnection;

/**
 * Implementation of a server using Winsock2 calls. Will listen for connections on a port, accept those connections
 * and send/recv data between the connections. Is implemented asynchronously using non-blocking sockets.
 * This class does not create any other threads.
 */
class WINDOWSSERVER_API Server : public quicktcp::server::IServer
{
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<async_cpp::workers::IManager> mgr, 
        std::shared_ptr<quicktcp::server::IResponder> responder);
    ~Server();

    virtual void shutdown();
    virtual std::future<async_cpp::async::AsyncResult> send(std::shared_ptr<utilities::ByteStream> stream); 
    virtual void waitForEvents();

private:
    /**
     * Create the socket that we listen on for new connections
     */
    void createServerSocket();
    /**
     * Create the asynchronous socket and functionality necessary for AcceptEx
     */
    void prepareForClientConnection(std::shared_ptr<ConnectOverlap> overlap);

    std::shared_ptr<IEventHandler> mEventHandler;
    std::shared_ptr<async_cpp::workers::IManager> mManager;
    std::shared_ptr<quicktcp::server::IResponder> mResponder;

    std::atomic<bool> mIsRunning;

    SOCKET mSocket; //this is the socket we listen for connections on
    HANDLE mIOCP;
    std::map<WSAEVENT, std::shared_ptr<ConnectOverlap>> mOverlaps;
};

}
}
}
}
