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

    void disconnected(ServerConnection& cnct)
    {
        cnct.disconnect();
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

    for(size_t i = 0; i < info.maxConnections(); ++i)
    {
        std::shared_ptr<Socket> socket(new Socket());
        std::shared_ptr<ConnectOverlap> overlap(new ConnectOverlap(std::move(socket), info.bufferSize()));
        prepareForClientConnection(overlap);
        mOverlaps.emplace(std::make_pair(overlap->hEvent, overlap));
    }
}

//------------------------------------------------------------------------------
void Server::createServerSocket()
{
    mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    //WSA startup successful, create address info
    struct addrinfo* result = nullptr, *ptr = nullptr, hints;

    ZeroMemory(&hints, sizeof(hints));
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

    CreateIoCompletionPort((HANDLE)mSocket, mIOCP, (ULONG_PTR)mSocket, 0);
}

//------------------------------------------------------------------------------
Server::~Server()
{
    shutdown();
}

//------------------------------------------------------------------------------
void Server::shutdown()
{
    WSARecvDisconnect(mSocket, 0);
    WSASendDisconnect(mSocket, 0);
    closesocket(mSocket);
    WSACleanup();
}

//------------------------------------------------------------------------------
void Server::waitForEvents()
{
    while(mIsRunning)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED overlap = 0;
        if(GetQueuedCompletionStatus(mIOCP, &bytes, &key, &overlap, WSA_INFINITE))
        {
            if(key == mSocket)
            {
                auto ioverlap = static_cast<IOverlap*>(overlap);
                switch (ioverlap->isConnect)
                {
                case true :
                    {
                        auto connectOverlap = static_cast<ConnectOverlap*>(overlap);
                        if(WSAGetOverlappedResult(connectOverlap->socket->socket(), connectOverlap, &connectOverlap->bytesRead, FALSE, 0))
                        {
                            //i/o complete?
                            connectOverlap->handleConnection(mIOCP, mEventHandler, mResponder);
                        }
                        else
                        {
                            if(WSA_IO_INCOMPLETE != GetLastError())
                            {
                                //wtf?
                                closesocket(connectOverlap->socket->socket());
                                throw(std::runtime_error("Error with connection socket"));
                            }
                        }
                    }
                    break;
                default:
                    {
                        auto sendOverlap = static_cast<SendOverlap*>(overlap);
                        sendOverlap->promise.set_value(async_cpp::async::AsyncResult());
                        delete sendOverlap;
                    }
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void Server::prepareForClientConnection(std::shared_ptr<ConnectOverlap> overlap)
{
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;

    size_t addrSize = 2*sizeof(SOCKADDR_STORAGE) + 32;

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR
            == WSAIoctl(mSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &acceptex_guid, sizeof(acceptex_guid), &pfnAcceptEx, sizeof(pfnAcceptEx),
                    &bytes, overlap.get(), nullptr))
    {
        throw(std::runtime_error("Failed to obtain AcceptEx() pointer"));
    }

    if(!pfnAcceptEx(mSocket, overlap->socket->socket(), overlap->buffer.get(),
            mInfo.bufferSize() - addrSize,
            sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &overlap->bytesRead,
            overlap.get()))
    {
        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            overlap->socket->close();
            overlap.reset();
            throw(std::runtime_error("AcceptEx Error()"));
        }
    }
}

//------------------------------------------------------------------------------
std::future<async_cpp::async::AsyncResult> Server::send(std::shared_ptr<utilities::ByteStream> stream)
{
    auto overlap = new SendOverlap(stream);

    DWORD flags = 0;
    DWORD bytesSent = 0;
    if(SOCKET_ERROR == WSASend(mSocket, &overlap->wsaBuffer, 1, &bytesSent, flags, overlap, 0))
    {
        if(WSA_IO_PENDING != WSAGetLastError())
        {
            overlap->promise.set_value(async_cpp::async::AsyncResult("Error sending data"));
        }
    }
    else
    {
        //send completed
        overlap->promise.set_value(async_cpp::async::AsyncResult());
    }

    return overlap->promise.get_future();
}

}
}
}
}
