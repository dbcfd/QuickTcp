#pragma once

#include "server/interface/Platform.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>

namespace quicktcp {

namespace workers {
class WorkerPool;
}

namespace utilities {
class ByteStream;
}

namespace server {
namespace iface {

class IServerConnection;

/**
 * Server interface. Platform specific implementations will implement this interface, allowing
 * common access to the platform servers
 */
class SERVER_INTERFACE_API IServer {
public:
    typedef std::function<void(std::shared_ptr<IServerConnection>)> ConnectionAdded;

    IServer(workers::WorkerPool* pool, ConnectionAdded caFunc);

    virtual void waitForEvents() = 0;
    virtual void disconnect() = 0;

    inline bool isRunning() const;
protected:
    std::string generateIdentifier();
    void addedConnection(std::shared_ptr<IServerConnection> connection);

    inline bool setRunning(const bool running);
    inline workers::WorkerPool* getWorkerPool() const;
private:
    virtual void shutdownImpl() = 0;
    workers::WorkerPool* mWorkerPool;
    std::chrono::time_point<std::chrono::system_clock> mLastTime;
    std::time_t mLastTimeT;
    std::atomic<bool> mRunning;
    ConnectionAdded mConnectionAdded;
    int mIdentifiersDuringTimeframe;
};

//inline implementations
bool IServer::isRunning() const
{
    return mRunning;
}

bool IServer::setRunning(const bool running)
{
    return mRunning.exchange(running);
}

workers::WorkerPool* IServer::getWorkerPool() const
{
    return mWorkerPool;
}

}
}
}