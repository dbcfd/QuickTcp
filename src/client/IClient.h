#pragma once

#include "client/Platform.h"
#include "client/ServerInfo.h"

#include <future>
#include <memory>
#include <thread>

namespace async_cpp {
namespace async {
class AsyncResult;
}
}

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

class CLIENT_API IClient {
public:
    IClient(const ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize);

    virtual std::future<async_cpp::async::AsyncResult> request(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void disconnect() = 0;

protected:
    std::shared_ptr<utilities::ByteStream> mAuthentication;
    size_t mBufferSize;
    std::thread mThread;
    ServerInfo mInfo;
};

//inline implementations
//------------------------------------------------------------------------------

}
}