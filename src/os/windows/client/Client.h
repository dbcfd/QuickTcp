#pragma once

#include "os/windows/client/Platform.h"
#include "os/windows/client/Socket.h"
#include "os/windows/client/Winsock2.h"

#include "client/IClient.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

/**
 * Winsock2 implementation of a client, which is a connection to some server. This class is implemented asynchronously
 * and will not create any additional threads.
 */
class WINDOWSCLIENT_API Client : public quicktcp::client::IClient
{
public:
    Client(const quicktcp::client::ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize,
        std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processStreamFunc);
	~Client();

    virtual std::future<async_cpp::async::AsyncResult> request(std::shared_ptr<utilities::ByteStream> stream);
    virtual void disconnect();

private:
    Client(const Client& other);

    void connect();

    class EventHandler;
    std::shared_ptr<EventHandler> mEventHandler;
    std::shared_ptr<Socket> mSocket;
    HANDLE mIOCP;
    bool mIsRunning;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
