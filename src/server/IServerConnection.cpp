#include "server/interface/IServerConnection.h"
#include "server/interface/SendTask.h"
#include "server/interface/ReceiveTask.h"

#include "workers/WorkerPool.h"

namespace quicktcp {
namespace server {
namespace iface {

IServerConnection::IServerConnection(const std::string& identifier, workers::WorkerPool* pool) 
    : mIdentifier(identifier), mWorkerPool(pool), 
    mReqToResp([](const utilities::ByteStream&) -> utilities::ByteStream {
        return utilities::ByteStream();
    } )
{

}

IServerConnection::~IServerConnection() 
{

}

std::future<bool> IServerConnection::send(const utilities::ByteStream& data)
{
    SendTask* sendTask = new SendTask(this, data);
    std::shared_ptr<workers::Task> task(sendTask);
    mWorkerPool->addWork(task);
    return sendTask->getResult();
}

void IServerConnection::receive()
{
    std::shared_ptr<workers::Task> task(new ReceiveTask(this, mReqToResp));
    mWorkerPool->addWork(task);
}

}
}
}