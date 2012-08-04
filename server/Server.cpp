#include "server/Server.h"

#ifdef WINDOWS
#include "server/windows/Server.h"
#else
#include "server/posix/Server.h"
#endif

namespace quicktcp {
namespace server {

Server::Server(workers::WorkerPool* pool, const unsigned int port, const unsigned int backlog, const unsigned int maxUnusedConnections, const iface::IServer::ConnectionAdded caFunc)
    : mThread(nullptr)
{
#ifdef WINDOWS
    mServer = new windows::Server(pool, port, backlog, maxUnusedConnections, caFunc);
#else
    mServer = new posix::Server(pool, port, backlog, maxUnusedConnections, caFunc);
#endif
}
Server::~Server()
{
    delete mServer;
    mThread->join();
    delete mThread;
}

void Server::waitForEvents(const bool runInOwnThread)
{
    if(runInOwnThread)
    {
        mThread = new std::thread(std::bind(&Server::serverWaitForEvents, this));
    }
    else
    {
        serverWaitForEvents();
    }
}

void Server::serverWaitForEvents()
{
    mServer->waitForEvents();
}

}
}
