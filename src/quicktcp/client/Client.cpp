#include "quicktcp/client/IClient.h"
#include "quicktcp/client/IProcessor.h"

#include "async/AsyncResult.h"

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
IClient::IClient(const ServerInfo& info, 
                 std::shared_ptr<utilities::ByteStream> authentication, 
                 const size_t bufferSize)
    : mInfo(info), mAuthentication(authentication), mBufferSize(bufferSize)
{
    
}

//------------------------------------------------------------------------------
IClient::~IClient()
{

}

//------------------------------------------------------------------------------
async_cpp::async::AsyncFuture IClient::process(std::shared_ptr<utilities::ByteStream> stream) const
{
    async_cpp::async::AsyncFuture result;
    if(mProcessor)
    {
        result = mProcessor->processResponse(stream);
    }
    else
    {
        std::promise<async_cpp::async::AsyncResult> promise;
        promise.set_value(async_cpp::async::AsyncResult());
        result = promise.get_future();
    }
    return result;
}

}
}