#include "server/IServer.h"
#include "server/IServerConnection.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
IServer::SendComplete::SendComplete()
{

}

//------------------------------------------------------------------------------
IServer::SendComplete::SendComplete(const std::string& error) : mError(error)
{

}

//------------------------------------------------------------------------------
IServer::IServer(const ServerInfo& info, std::shared_ptr<workers::Manager> manager, std::function<std::unique_ptr<IServerConnection>()> createConnectionFunc)
    : mInfo(info), mManager(manager), mCreateConnectionFunc(createConnectionFunc), mIsRunning(true)
{
    
}

//------------------------------------------------------------------------------
void IServer::shutdown()
{
    bool wasRunning = mIsRunning.exchange(false);

    if(wasRunning)
    {
        performShutdown();
    }
}

//------------------------------------------------------------------------------
std::future<IServer::SendComplete> IServer::send(std::shared_ptr<utilities::ByteStream> stream)
{
    std::unique_ptr<std::promise<SendComplete>> promise(new std::promise<SendComplete>());
    std::future<SendComplete> future = promise->get_future();

    auto func = std::bind([this, stream](std::unique_ptr<std::promise<SendComplete>> promise)->void {
        performSend(std::move(promise), stream);
    }, std::move(promise));

    return future;
}

//------------------------------------------------------------------------------
std::unique_ptr<IServerConnection> IServer::addConnection()
{
    return mCreateConnectionFunc();
}

}
}