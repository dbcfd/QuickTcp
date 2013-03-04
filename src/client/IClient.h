#pragma once

#include "client/Platform.h"
#include "client/ServerInfo.h"

#include <future>
#include <memory>

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
    class IListener 
    {
    public:
        virtual void receive(std::shared_ptr<utilities::ByteStream> stream) = 0;
        virtual void serverDisconnected() = 0;
    };

    IClient(const ServerInfo& info, std::shared_ptr<IListener> listener);

    virtual std::future<async_cpp::async::AsyncResult> send(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void disconnect() = 0;
    virtual void waitForEvents() = 0;

    void sendDataToListener(std::shared_ptr<utilities::ByteStream> stream);

protected:
    std::shared_ptr<IListener> mListener;
    ServerInfo mInfo;
};

//inline implementations
//------------------------------------------------------------------------------

}
}