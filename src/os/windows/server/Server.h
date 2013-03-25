#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/Winsock2.h"

#include "server/IServer.h"

#include <atomic>
#include <condition_variable>
#include <vector>

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
    virtual void waitForEvents();

    void send(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream); 

    /**
     * Create the asynchronous socket and functionality necessary for AcceptEx
     */
    void prepareForClientConnection(ConnectOverlap& overlap);

    inline HANDLE getIOCompletionPort() const;

private:
    /**
     * Create the socket that we listen on for new connections
     */
    void createServerSocket();

    std::shared_ptr<IEventHandler> mEventHandler;

    std::atomic<bool> mRunning;
    bool mWaitingForEvents;
    std::mutex mMutex;

    SOCKET mSocket; //this is the socket we listen for connections on
    HANDLE mIOCP;
    std::vector<std::shared_ptr<ConnectOverlap>> mOverlaps;
    std::condition_variable mShutdownSignal;
};

//inline implementations
//------------------------------------------------------------------------------
HANDLE Server::getIOCompletionPort() const
{
    return mIOCP;
}

}
}
}
}
