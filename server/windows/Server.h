#pragma once

#include "server/windows/Platform.h"

#include <future>
#include <map>
#include <mutex>

#include "server/interface/IServer.h"

#include "server/windows/Overlap.h"
#include "server/windows/Socket.h"
#include "server/windows/Winsock2.h"

namespace quicktcp {
namespace server {

namespace iface {
class ICallback;
class IServerConnection;
}

namespace windows {

/**
 * Implementation of a server using Winsock2 calls. Will listen for connections on a port, accept those connections
 * and send/recv data between the connections. Is implemented asynchronously using non-blocking sockets.
 * This class does not create any other threads.
 */
class SERVER_WINDOWS_API Server: public iface::IServer, public Overlap<Server>
{
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(workers::WorkerPool* pool, const unsigned int port, const unsigned int backlog, const unsigned int unusedConnections, iface::IServer::ConnectionAdded caFunc);
    ~Server();

    virtual void disconnect();
    virtual void waitForEvents();    

private:
    /**
     * Receive data from a connection that has triggered a receive event on the handle
     */
    void handleDataFromConnection(HANDLE handleId);
    /**
     * The listening socket has a connection ready to be added.
     */
    void addNewConnection();
    /**
     * Wait until a handle has some event occurring (e.g. connection, send, recv)
     */
    HANDLE performWaitForEvents();
    /**
     * Create the socket that we listen on for new connections
     */
    void createServerSocket(const unsigned int port, const unsigned int backlog);
    /**
     * Create the asynchronous socket and functionality necessary for AcceptEx
     */
    void prepareForServerConnection();
    /**
     * Handle a connection being closed
     */
    void connectionClosed(HANDLE handle);

    Socket* mServerSocket; //this is the socket we listen for connections on
    Socket* mIncomingSocket; //this is the socket that will be used when a connection occurs, as specified in AcceptEx

    std::map<WSAEVENT, std::shared_ptr<iface::IServerConnection>> mConnections;
    std::mutex mConnectionsMutex;
    std::vector<WSAEVENT> mHandles;

    //buffer used when connections accepted
    char mAcceptExBuffer[2 * (sizeof(SOCKADDR_STORAGE) + 16)];
    std::promise<bool> mDoneWaitingForEvents;
    std::atomic<bool> mIgnoreClose;
    unsigned int mUnusedConnections;
    unsigned int mMaxUnusedConnections;
    iface::IServer::ConnectionAdded mConnectionAdded;

};

}
}
}
