#pragma once

#include "os/windows/client/Platform.h"
#include "os/windows/client/Socket.h"
#include "os/windows/client/Winsock2.h"

#include "client/IClient.h"

#include <atomic>
#include <map>

namespace async_cpp {
namespace workers {
class IManager;
}
}

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
    Client(std::shared_ptr<async_cpp::workers::IManager> mgr, 
        const quicktcp::client::ServerInfo& info,
        std::shared_ptr<IListener> listener,
        const size_t allocationSize = 1024);
	~Client();

    virtual std::future<async_cpp::async::AsyncResult> send(std::shared_ptr<utilities::ByteStream> stream);
    virtual void disconnect();
    virtual void waitForEvents();

    std::shared_ptr<utilities::ByteStream> stream(size_t nbBytes) const;
    void completeSend();

    struct SendOverlap;
private:
    Client(const Client& other);
    struct ReceiveOverlap;

    void prepareClientToReceiveData();
    void connect(const quicktcp::client::ServerInfo& info);

    std::shared_ptr<async_cpp::workers::IManager> mManager;
    Socket mSocket;
    WSABUF mBuffer;
    std::shared_ptr<char> mAllocatedStorage;
    size_t mAllocationSize;
    std::atomic<bool> mConnected;
    std::shared_ptr<ReceiveOverlap> mReceiveOverlap;
    std::atomic<size_t> mSendsOutstanding;
    std::condition_variable mSendsOutstandingSignal;
};

//inline implementations
//------------------------------------------------------------------------------

}
}
}
}
