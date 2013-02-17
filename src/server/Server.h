#pragma once

#include "server/Platform.h"

#include "server/interface/IServer.h"

#include <thread>

namespace quicktcp {
namespace server {

/**
 * Generic implementation of a server. Receives data from a platform implementation (posix/winsock) and then
 * passes that information along to a worker.
 */
class SERVER_API Server {
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(workers::WorkerPool* pool, unsigned int port, const unsigned int backlog, const unsigned int maxUnusedConnections, const iface::IServer::ConnectionAdded caFunc);
    ~Server();

    /**
     * Blocks the current thread, waiting until an event occurs. Events include connection attempts, sending/receiving
     * data, and shutdown.
     */
    void waitForEvents(const bool runInOwnThread);
    /**
     * Shutdown this server, closing all connections.
     */
    inline void disconnect();

    inline bool isRunning() const;

private:
    void serverWaitForEvents();

    iface::IServer* mServer;
    std::thread* mThread;
};

//inline implementations
void Server::disconnect() 
{
    mServer->disconnect();
}

bool Server::isRunning() const
{
    return mServer->isRunning();
}

}
}
