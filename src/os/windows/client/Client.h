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
        const size_t bufferSize);
	~Client();

    virtual async_cpp::async::AsyncFuture request(std::shared_ptr<utilities::ByteStream> stream);
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
