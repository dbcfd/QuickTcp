#include "server/IServerConnection.h"
#include "server/IResponder.h"

#include <assert.h>

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
IServerConnection::IServerConnection(std::shared_ptr<IResponder> responder) : mResponder(responder)
{
    assert(nullptr != responder);
}

//------------------------------------------------------------------------------
IServerConnection::~IServerConnection()
{

}

//------------------------------------------------------------------------------
std::future<async_cpp::async::AsyncResult> IServerConnection::getResponse(std::shared_ptr<utilities::ByteStream> stream)
{
    return mResponder->respond(stream);
}

}
}