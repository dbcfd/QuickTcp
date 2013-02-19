#include "server/IServerConnection.h"
#include "server/IServer.h"

#include "workers/Manager.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
IServerConnection::Response::Response(std::shared_ptr<utilities::ByteStream> stream) : mStream(stream)
{

}

//------------------------------------------------------------------------------
IServerConnection::Response::Response(const std::string& error) : mError(error)
{

}

//------------------------------------------------------------------------------
IServerConnection::IServerConnection(std::shared_ptr<IServer> server) : mServer(server)
{

}

//------------------------------------------------------------------------------
std::future<IServerConnection::ResponseComplete> IServerConnection::respond(std::shared_ptr<utilities::ByteStream> stream)
{
    std::unique_ptr<std::promise<ResponseComplete>> promise(new std::promise<ResponseComplete>());
    std::future<ResponseComplete> future = promise->get_future();

    auto func = std::bind([this, stream](std::unique_ptr<std::promise<ResponseComplete>> promise)->void {
        Response resp = determineResponse(stream);
        auto sendFunc = std::bind([this](std::unique_ptr<std::promise<ResponseComplete>> promise, const Response& response)->void {
            if(!response.error().empty())
            {
                std::future<IServer::SendComplete> future = mServer->send(response.stream());
                auto waitFunc = std::bind([](std::unique_ptr<std::promise<ResponseComplete>> promise, std::future<IServer::SendComplete> future)->void{
                    future.wait();
                    promise->set_value(future.get().error());
                }, std::move(promise), std::move(future));
                mServer->manager()->run(waitFunc);
            }
            else
            {
                promise->set_value(response.error());
            }
        }, std::move(promise), std::move(resp));
        mServer->manager()->run(sendFunc);
    }, std::move(promise));
    mServer->manager()->run(func);

    return future;
}

}
}