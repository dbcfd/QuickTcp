#include "client/IClient.h"

#include "workers/Manager.h"

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
IClient::SendResult::SendResult()
{

}

//------------------------------------------------------------------------------
IClient::SendResult::SendResult(const std::string& err) : mError(err)
{

}

//------------------------------------------------------------------------------
IClient::IClient(std::shared_ptr<workers::Manager> manager, 
                 const ServerInfo& info, 
                 std::function<void(std::shared_ptr<utilities::ByteStream>)> onReceive)
    : mManager(manager), mOnReceive(onReceive), mInfo(info)
{

}

//------------------------------------------------------------------------------
std::future<IClient::SendResult> IClient::send(std::shared_ptr<utilities::ByteStream> stream)
{
    std::shared_ptr<std::promise<SendResult>> promise(new std::promise<SendResult>());

    auto func = [this, promise, stream]()-> void {
        performSend(promise, stream);
    };

    mManager->run(func);

    return promise->get_future();
}

//------------------------------------------------------------------------------
void IClient::receive(std::shared_ptr<utilities::ByteStream> stream) const
{
    mManager->run([this, stream]()->void {
        mOnReceive(stream);
    } );
}

}
}