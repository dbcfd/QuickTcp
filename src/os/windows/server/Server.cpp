#include "os/windows/server/Server.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/ReceiveOverlap.h"
#include "os/windows/server/ResponseOverlap.h"
#include "os/windows/server/SendOverlap.h"
#include "os/windows/server/Socket.h"

#include "async/AsyncResult.h"

#include "server/IResponder.h"

#include "utilities/ByteStream.h"

#include "workers/IManager.h"
#include "workers/BasicTask.h"

#include <assert.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
class EventHandler : public IEventHandler {
public:
    EventHandler(Server& srvr, std::shared_ptr<quicktcp::server::IResponder> resp, std::shared_ptr<async_cpp::workers::IManager> mgr) 
        : server(srvr), responder(resp), manager(mgr)
    {

    }

    virtual void authenticateConnection(std::shared_ptr<utilities::ByteStream> stream, ReceiveOverlap* overlap)
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, overlap]()->void {
                bool authenticated = false;
                try {
                    authenticated = responder->authenticateConnection(stream);
                    PostQueuedCompletionStatus(server.getIOCompletionPort(), 0, 0, overlap);
                }
                catch(std::exception& ex)
                {
                    reportError(ex.what());
                    overlap->disconnect();
                }
            }
        ) ) );

    }

    virtual void queueAccept(ConnectOverlap& overlap)
    {
        server.prepareForClientConnection(overlap);
    }

    virtual void createResponse(std::shared_ptr<utilities::ByteStream> stream, ResponseOverlap* overlap)
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, overlap]()->void {
                try {
                    overlap->mResult = responder->respond(stream);
                }
                catch(std::exception& ex)
                {
                    //failure
                    overlap->mResult = async_cpp::async::AsyncResult(ex.what());
                }
                PostQueuedCompletionStatus(server.getIOCompletionPort(), 0, 0, overlap);
            }
        ) ) );
    }

    virtual void reportError(const std::string& error)
    {
        //TODO
    }

    virtual void connectionClosed()
    {
        responder->handleConnectionClosed();
    }

    virtual void sendResponse(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream)
    {
        server.send(sckt, stream);
    }

    virtual size_t connectBufferSize() const
    {
        return size_t(100);
    }

    virtual size_t receiveBufferSize() const
    {
        return size_t(2000);
    }

    virtual size_t responseBufferSize() const 
    {
        return size_t(100);
    }

    Server& server;
    std::shared_ptr<quicktcp::server::IResponder> responder;
    std::shared_ptr<async_cpp::workers::IManager> manager;

};

//------------------------------------------------------------------------------
Server::Server(const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<async_cpp::workers::IManager> mgr, 
        std::shared_ptr<quicktcp::server::IResponder> responder) :
    quicktcp::server::IServer(info), mManager(mgr), mResponder(responder), mIsRunning(true)
{
    mEventHandler = std::shared_ptr<IEventHandler>(new EventHandler(*this, mResponder, mManager));
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
            overlap = std::shared_ptr<ConnectOverlap>(new ConnectOverlap(std::move(socket), mEventHandler, mIOCP));
            prepareForClientConnection(*overlap);
            mOverlaps.emplace_back(overlap);
        }
        catch(std::runtime_error&)
        {
            mEventHandler->reportError(std::string("Failed to build ConnectOverlap ") + std::to_string(i));
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

    auto port = std::to_string(mInfo.port());

    int iResult = getaddrinfo("127.0.0.1", port.c_str(), &hints, &result);

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
        auto error = std::string("Bind error: ") + std::to_string(iResult);
        closesocket(mSocket);
        WSACleanup();
        throw(std::runtime_error(error));
    }

    iResult = listen(mSocket, mInfo.maxBacklog());
    if(SOCKET_ERROR == iResult)
    {
        auto error = std::string("Listen error: ") + std::to_string(iResult);
        closesocket(mSocket);
        WSACleanup();
        throw(std::runtime_error(error));
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
        PostQueuedCompletionStatus(mIOCP, 0, 0, 0);
        WSARecvDisconnect(mSocket, 0);
        WSASendDisconnect(mSocket, 0);
        closesocket(mSocket);
        for(auto overlap : mOverlaps)
        {
            overlap->closeEvent();
            overlap->mSocket->close();
        }
        mManager->waitForTasksToComplete();
        CloseHandle(mIOCP);
        mOverlaps.clear();
        WSACleanup();
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
                ioverlap->handleIOCompletion(bytes);
                if(ioverlap->requiresDeletion())
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
    if(mSocket != INVALID_SOCKET && overlap.mSocket->isValid())
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
            mEventHandler->reportError("Failed to obtain AcceptEx() pointer");
        }

        if(!pfnAcceptEx(mSocket, overlap.mSocket->socket(), overlap.mBuffer.get(),
                0, //mInfo.bufferSize() - addrSize,
                sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &(overlap.mBytes),
                &overlap))
        {
            int lasterror = WSAGetLastError();

            if(WSA_IO_PENDING != lasterror)
            {
                overlap.mSocket->close();            
                mEventHandler->reportError("AcceptEx Error()");
            }
        }
        else
        {
            overlap.handleConnection();
        }
    }
}

//------------------------------------------------------------------------------
void Server::send(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream)
{
    if(sckt->isValid())
    {
        auto overlap = new SendOverlap(sckt, mEventHandler, stream);

        if(SOCKET_ERROR == WSASend(sckt->socket(), &overlap->mWsaBuffer, 1, &overlap->mBytes, overlap->mFlags, overlap, 0))
        {
            int lastError = WSAGetLastError();
            if(WSA_IO_PENDING != lastError)
            {
                mEventHandler->reportError("Error sending data");
                delete overlap;
            }
        }
        else
        {
            overlap->completeSend(); //no delete required, i/o completion will be hit
        }
    }
}

}
}
}
}
