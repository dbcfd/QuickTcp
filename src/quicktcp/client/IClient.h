#pragma once

#include "quicktcp/client/Platform.h"
#include "quicktcp/client/ServerInfo.h"

#include "async/Async.h"

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

class IProcessor;

class CLIENT_API IClient {
public:
    IClient(const ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize);
    virtual ~IClient();

    virtual async_cpp::async::AsyncFuture request(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void disconnect() = 0;

    inline size_t bufferSize() const;
    inline void setProcessor(std::shared_ptr<IProcessor> processor);

protected:
    async_cpp::async::AsyncFuture process(std::shared_ptr<utilities::ByteStream> stream) const;

    std::shared_ptr<utilities::ByteStream> mAuthentication;
    std::shared_ptr<IProcessor> mProcessor;
    size_t mBufferSize;
    std::thread mThread;
    ServerInfo mInfo;
};

//inline implementations
//------------------------------------------------------------------------------
size_t IClient::bufferSize() const
{
    return mBufferSize;
}

//------------------------------------------------------------------------------
void IClient::setProcessor(std::shared_ptr<IProcessor> processor)
{
    mProcessor = processor;
}

}
}