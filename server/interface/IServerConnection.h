#pragma once

#include "utilities/ByteStream.h"
#include "utilities/RequestToResponse.h"

#include "workers/Task.h"

#include "server/interface/Platform.h"

#include <functional>
#include <future>
#include <string>

namespace quicktcp {

namespace workers {
class WorkerPool;
}

namespace server {
namespace iface {

class ReceiveTask;
class SendTask;

class SERVER_INTERFACE_API IServerConnection {
public:
    IServerConnection(const std::string& identifier, workers::WorkerPool* pool);
    virtual ~IServerConnection();

    std::future<bool> send(const utilities::ByteStream& data);
    void receive();

    virtual void close() = 0;

    inline void setReceiveHandler(utilities::RequestToResponse reqToResp);
    inline const std::string& getIdentifier() const;
    inline workers::WorkerPool* getWorkerPool() const;

    bool operator<(const IServerConnection& other)
    {
        return mIdentifier < other.mIdentifier;
    }

    inline bool connected() const;
protected:
    virtual bool blockingSend(const utilities::ByteStream& data) = 0;
    virtual utilities::ByteStream blockingReceive() = 0;

    inline void disconnect();
private:
    std::string mIdentifier;
    workers::WorkerPool* mWorkerPool;
    utilities::RequestToResponse mReqToResp;
    std::atomic<bool> mConnected;
    
    friend class SendTask;
    friend class ReceiveTask;
};

//inline implementations
const std::string& IServerConnection::getIdentifier() const
{
    return mIdentifier;
}

workers::WorkerPool* IServerConnection::getWorkerPool() const
{
    return mWorkerPool;
}

void IServerConnection::setReceiveHandler(utilities::RequestToResponse reqToResp)
{
    mReqToResp = reqToResp;
}

bool IServerConnection::connected() const
{
    return mConnected;
}

void IServerConnection::disconnect()
{
    mConnected = false;
}

}
}
}