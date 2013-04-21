#include "os/windows/server/Server.h"
#include "os/windows/server/Overlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ConnectCompleter.h"
#include "os/windows/server/ReceiveCompleter.h"
#include "os/windows/server/ResponseCompleter.h"
#include "os/windows/server/SendCompleter.h"
#include "os/windows/server/Socket.h"

#include "async/AsyncResult.h"

#include "server/IResponder.h"

#include "utilities/ByteStream.h"

#include "workers/IManager.h"
#include "workers/BasicTask.h"

#include <assert.h>
#include <chrono>
#include <string>
#include <map>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//TODO: These should be configurable
static const size_t CONNECT_BUFFER_SIZE(200);
static const size_t RECEIVE_BUFFER_SIZE(2048);
static const size_t RESPONSE_BUFFER_SIZE(200);

//------------------------------------------------------------------------------
class EventHandler : public IEventHandler {
public:
    EventHandler(Server& srvr, std::shared_ptr<quicktcp::server::IResponder> resp, std::shared_ptr<async_cpp::workers::IManager> mgr) 
        : server(srvr), responder(resp), manager(mgr)
    {

    }

    virtual void authenticateConnection(std::shared_ptr<utilities::ByteStream> stream, Overlap* overlap) final
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, overlap]()->void {
                auto completer = std::static_pointer_cast<ReceiveCompleter>(overlap->completer());
                try {
                    if(responder->authenticateConnection(stream))
                    {
                        completer->prepareToReceive(*overlap);
                    }
                    else
                    {
                        completer->disconnect();
                    }
                }
                catch(std::exception& ex)
                {
                    reportError(ex.what());
                    completer->disconnect();
                }
            }
        ) ) );
    }

    virtual void queueAccept(std::shared_ptr<ConnectCompleter> completer) final
    {
        auto overlap = new Overlap(completer, connectBufferSize());
        if(server.getIOCompletionPort() != CreateIoCompletionPort((HANDLE)overlap->winsocket(), server.getIOCompletionPort(), (ULONG_PTR)overlap, 0))
        {
            auto error = std::string("CreateIoCompletionPort Error: ") + std::to_string(WSAGetLastError());
            throw(std::runtime_error(error));
        }
        server.prepareForClientConnection(*overlap);
    }

    virtual void createResponse(std::shared_ptr<utilities::ByteStream> stream, Overlap* overlap)
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, overlap]()->void {
                auto completer = std::static_pointer_cast<ResponseCompleter>(overlap->completer());
                try {
                    completer->setResult(std::move(responder->respond(stream)));
                }
                catch(std::exception& ex)
                {
                    //failure
                    completer->setResult(std::move(async_cpp::async::AsyncResult(ex.what())));
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
        return CONNECT_BUFFER_SIZE;
    }

    virtual size_t receiveBufferSize() const
    {
        return RECEIVE_BUFFER_SIZE;
    }

    virtual size_t responseBufferSize() const 
    {
        return RESPONSE_BUFFER_SIZE;
    }

    Server& server;
    std::shared_ptr<quicktcp::server::IResponder> responder;
    std::shared_ptr<async_cpp::workers::IManager> manager;

};

//------------------------------------------------------------------------------
Server::Server(const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<async_cpp::workers::IManager> mgr, 
        std::shared_ptr<quicktcp::server::IResponder> responder) :
    quicktcp::server::IServer(info, mgr, responder), mRunning(true), mWaitingForEvents(false)
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

    for(size_t i = 0; i < info.maxConnections(); ++i)
    {
        mConnectionSockets.emplace_back(new Socket());
        auto completer = std::make_shared<ConnectCompleter>(mConnectionSockets.back(), mEventHandler);
        mEventHandler->queueAccept(completer);
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

    auto iResult = getaddrinfo("127.0.0.1", port.c_str(), &hints, &result);

    if(iResult != 0)
    {
        WSACleanup();
        throw(std::runtime_error("getaddrinfo Error"));
    }

    //getaddrinfo successful, create socket
    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    BOOL opt = TRUE;
    setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(BOOL));

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

    iResult = listen(mSocket, (int)mInfo.maxBacklog());
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
}

//------------------------------------------------------------------------------
void Server::shutdown()
{
    bool waitingForEvents = mWaitingForEvents;
    bool wasRunning = mRunning.exchange(false);
    if(wasRunning)
    {
        for(auto sckt : mConnectionSockets)
        {
            sckt->close();
        }
        //if wait for events was never called, we need to clean up overlaps
        if(!waitingForEvents)
        {
            cleanupOverlaps();
        }
        {         
            std::unique_lock<std::mutex> lock(mMutex);
            mShutdownSignal.wait(lock, [this]()->bool {
                return !mWaitingForEvents;
            } );
        }
        mManager->waitForTasksToComplete();
        //finish clean up
        closesocket(mSocket);
        CloseHandle(mIOCP);
        WSACleanup();
        mShutdownSignal.notify_all();
    }
}

//------------------------------------------------------------------------------
void Server::waitForEvents()
{
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mWaitingForEvents = true;
    }
    while(mRunning)
    {
        OVERLAPPED_ENTRY overlaps[10];
        ULONG overlapsReturned = 0;
        if(GetQueuedCompletionStatusEx(mIOCP, overlaps, sizeof(overlaps), &overlapsReturned, WSA_INFINITE, FALSE))
        {
            for(auto idx = ULONG(0); idx < overlapsReturned; ++idx)
            {
                if(nullptr != overlaps[idx].lpOverlapped)
                {
                    auto overlap = static_cast<Overlap*>(overlaps[idx].lpOverlapped);
                    overlap->handleIOCompletion(overlaps[idx].dwNumberOfBytesTransferred);
                    if(overlap->readyForDeletion())
                    {
                        delete overlap;
                    }
                }
            }
        }
    }   

    cleanupOverlaps();

    {
        std::unique_lock<std::mutex> lock(mMutex);
        mWaitingForEvents = false;
    }
    mShutdownSignal.notify_all();
}

//------------------------------------------------------------------------------
void Server::cleanupOverlaps()
{
    OVERLAPPED_ENTRY overlaps[10];
    ULONG overlapsReturned = 0;
    while(GetQueuedCompletionStatusEx(mIOCP, overlaps, sizeof(overlaps), &overlapsReturned, WSA_INFINITE, FALSE))
    {
        for(auto idx = ULONG(0); idx < overlapsReturned; ++idx)
        {
            if(nullptr != overlaps[idx].lpOverlapped)
            {
                auto overlap = static_cast<Overlap*>(overlaps[idx].lpOverlapped);
                if(overlap->readyForDeletion())
                {
                    delete overlap;
                }
                else
                {
                    overlap->shutdown();
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void Server::prepareForClientConnection(Overlap& overlap)
{
    if(mSocket != INVALID_SOCKET && overlap.socket()->isValid())
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

        if(!overlap.queueAcceptEx(pfnAcceptEx, mSocket))
        {
            int lasterror = WSAGetLastError();

            if(WSA_IO_PENDING != lasterror)
            {
                overlap.socket()->close();            
                mEventHandler->reportError("AcceptEx Error()");
            }
        }
        else
        {
            overlap.handleIOCompletion(bytes);
        }
    }
}

//------------------------------------------------------------------------------
void Server::send(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream)
{
    if(sckt->isValid())
    {
        auto completer = std::make_shared<SendCompleter>(sckt, mEventHandler, stream->size());
        auto overlap = new Overlap(completer, stream);

        //if socket error, need to see if it was just pending
        //completed sends will be reported via i/o completion
        if(SOCKET_ERROR == overlap->queueSend())
        {
            int lastError = WSAGetLastError();
            if(WSA_IO_PENDING != lastError)
            {
                mEventHandler->reportError(std::string("Error sending data") + std::to_string(lastError));
                overlap->shutdown();
            }
        }
    }
}

}
}
}
}
