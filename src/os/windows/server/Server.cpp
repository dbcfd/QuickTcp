#include "os/windows/server/Server.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/ServerConnection.h"
#include "os/windows/server/SendOverlap.h"
#include "os/windows/server/Socket.h"

#include "async/AsyncResult.h"

#include "utilities/ByteStream.h"

#include "workers/IManager.h"
#include "workers/BasicTask.h"

#include <iostream>
#include <sstream>

#include <assert.h>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
class EventHandler : public IEventHandler {
public:
    EventHandler(Server& srvr, std::shared_ptr<async_cpp::workers::IManager> mgr) : server(srvr), manager(mgr)
    {

    }

    void queueAccept(ConnectOverlap& overlap)
    {
        server.prepareForClientConnection(overlap);
    }

    void handleResponse(std::future<async_cpp::async::AsyncResult>& response) 
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, &response]() -> void {
                auto result = response.get();
                if(!result.wasError())
                {
                    server.send(std::static_pointer_cast<utilities::ByteStream>(result.result()));
                }
            }
        ) ) );
    }

    Server& server;
    std::shared_ptr<async_cpp::workers::IManager> manager;

};

//------------------------------------------------------------------------------
Server::Server(const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<async_cpp::workers::IManager> mgr, 
        std::shared_ptr<quicktcp::server::IResponder> responder) :
    quicktcp::server::IServer(info), mManager(mgr), mResponder(responder), mIsRunning(true)
{
    mEventHandler = std::shared_ptr<IEventHandler>(new EventHandler(*this, mManager));
    assert(nullptr != mgr);
    assert(nullptr != responder);

    //startup winsock
    WSAData wsaData;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw(std::runtime_error("WSAStartup Error"));
    }

    createServerSocket();

    mOverlaps.reserve(info.maxConnections());
    for(size_t i = 0; i < info.maxConnections(); ++i)
    {
        std::shared_ptr<Socket> socket(new Socket());
        std::shared_ptr<ConnectOverlap> overlap;
        try {
            overlap = std::shared_ptr<ConnectOverlap>(new ConnectOverlap(mIOCP, mEventHandler, mResponder, std::move(socket), info.bufferSize()));
            prepareForClientConnection(*overlap);
            mOverlaps.emplace_back(overlap);
        }
        catch(std::runtime_error&)
        {
            //todo logging
            std::cout << "error building connection " << i << std::endl;
        }
    }
}

//------------------------------------------------------------------------------
void Server::createServerSocket()
{
    mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    //WSA startup successful, create address info
    struct addrinfo* result = nullptr, *ptr = nullptr, hints;

    SecureZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream sstr;
    sstr << mInfo.port();

    int iResult = getaddrinfo("127.0.0.1", sstr.str().c_str(), &hints, &result);

    if(iResult != 0)
    {
        WSACleanup();
        throw(std::runtime_error("getaddrinfo Error"));
    }

    //getaddrinfo successful, create socket
    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    CreateIoCompletionPort((HANDLE)mSocket, mIOCP, (ULONG_PTR)mSocket, 0);

    //socket creation successful, bind to socket
    iResult = bind(mSocket, result->ai_addr, (int) result->ai_addrlen);
    freeaddrinfo(result);

    if(SOCKET_ERROR == iResult)
    {
        std::stringstream sstr;
        sstr << "bind Error " << WSAGetLastError();
        closesocket(mSocket);
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    if(SOCKET_ERROR == listen(mSocket, mInfo.maxBacklog()))
    {
        std::stringstream sstr;
        sstr << "listen Error " << WSAGetLastError();
        closesocket(mSocket);
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }
}

//------------------------------------------------------------------------------
Server::~Server()
{
    shutdown();
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mShutdownSignal.wait(lock, [this]()->bool {
        return mOverlaps.empty();
    } );
}

//------------------------------------------------------------------------------
void Server::shutdown()
{
    bool wasRunning = mIsRunning.exchange(false);
    if(wasRunning)
    {
        ULONG_PTR key = 0;
        PostQueuedCompletionStatus(mIOCP, 0, key, 0);
        WSARecvDisconnect(mSocket, 0);
        WSASendDisconnect(mSocket, 0);
        closesocket(mSocket);
        for(auto overlap : mOverlaps)
        {
            overlap->socket->close();
        }
        WSACleanup();
        mOverlaps.clear();
        mShutdownSignal.notify_all();
    }
}

//------------------------------------------------------------------------------
void Server::waitForEvents()
{
    while(mIsRunning)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED overlap = 0;
        if(GetQueuedCompletionStatus(mIOCP, &bytes, &key, &overlap, WSA_INFINITE) && mIsRunning)
        {
            if(nullptr != overlap)
            {
                auto ioverlap = static_cast<IOverlap*>(overlap);
                if(ioverlap->handleIOCompletion((SOCKET)key, bytes))
                {
                    delete ioverlap;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void Server::prepareForClientConnection(ConnectOverlap& overlap)
{
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;

    size_t addrSize = 2*sizeof(SOCKADDR_STORAGE) + 32;

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR
            == WSAIoctl(mSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &acceptex_guid, sizeof(acceptex_guid), &pfnAcceptEx, sizeof(pfnAcceptEx),
                    &bytes, &overlap, 0))
    {
        int lasterror = WSAGetLastError();
        throw(std::runtime_error("Failed to obtain AcceptEx() pointer"));
    }

    if(!pfnAcceptEx(mSocket, overlap.socket->socket(), overlap.buffer.get(),
            0, //mInfo.bufferSize() - addrSize,
            sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &(overlap.bytes),
            &overlap))
    {
        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            overlap.socket->close();            
            throw(std::runtime_error("AcceptEx Error()"));
        }
    }
    else
    {
        overlap.handleConnection();
    }
}

//------------------------------------------------------------------------------
std::future<async_cpp::async::AsyncResult> Server::send(std::shared_ptr<utilities::ByteStream> stream)
{
    auto overlap = new SendOverlap(mSocket, stream);
    auto future = overlap->promise.get_future();

    if(SOCKET_ERROR == WSASend(mSocket, &overlap->wsaBuffer, 1, &overlap->bytes, overlap->flags, overlap, 0))
    {
        if(WSA_IO_PENDING != WSAGetLastError())
        {
            overlap->promise.set_value(async_cpp::async::AsyncResult("Error sending data"));
        }
    }
    else
    {
        overlap->completeSend(overlap->bytes);
        delete overlap;
    }

    return future;
}

}
}
}
}
