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
#include <chrono>
#include <string>
#include <map>

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

    virtual void authenticateConnection(std::shared_ptr<utilities::ByteStream> stream, ReceiveOverlap& overlap)
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, &overlap]()->void {
                try {
                    if(responder->authenticateConnection(stream))
                    {
                        overlap.prepareToReceive();
                    }
                    else
                    {
                        overlap.disconnect();
                    }
                }
                catch(std::exception& ex)
                {
                    reportError(ex.what());
                    overlap.disconnect();
                }
            }
        ) ) );
    }

    virtual void markForDeletion(IOverlap& overlap)
    {
        auto now = std::chrono::system_clock::now();
        std::unique_lock<std::mutex> lock(mutex);
        if(!overlapsForDeletion.empty())
        {
            auto deletionCutoffTime = now - std::chrono::seconds(30);
            if(overlapsForDeletion.begin()->first < deletionCutoffTime)
            {
                auto endItr = overlapsForDeletion.lower_bound(deletionCutoffTime);
                for(auto itr = overlapsForDeletion.begin(); itr != endItr; ++itr)
                {
                    delete itr->second;
                }
                overlapsForDeletion.erase(overlapsForDeletion.begin(), endItr);
            }
        }
        overlapsForDeletion.insert(std::make_pair(std::chrono::system_clock::now(), &overlap));
    }

    virtual void deleteMarkedOverlaps()
    {
        std::unique_lock<std::mutex> lock(mutex);
        for(auto& kv : overlapsForDeletion)
        {
            delete kv.second;
        }
        overlapsForDeletion.clear();
    }

    virtual void queueAccept(ConnectOverlap& overlap)
    {
        server.prepareForClientConnection(overlap);
    }

    virtual void createResponse(std::shared_ptr<utilities::ByteStream> stream, ResponseOverlap& overlap)
    {
        manager->run(std::shared_ptr<async_cpp::workers::Task>(new async_cpp::workers::BasicTask(
            [this, stream, &overlap]()->void {
                try {
                    overlap.mResult = responder->respond(stream);
                }
                catch(std::exception& ex)
                {
                    //failure
                    overlap.mResult = async_cpp::async::AsyncResult(ex.what());
                }
                PostQueuedCompletionStatus(server.getIOCompletionPort(), 0, 0, &overlap);
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
    std::mutex mutex;
    std::map<std::chrono::system_clock::time_point, IOverlap*> overlapsForDeletion;

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
}

//------------------------------------------------------------------------------
void Server::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    if(wasRunning)
    {
        CloseHandle(mIOCP);
        for(auto& overlap : mOverlaps)
        {
            overlap->waitForDisconnect();
            overlap->mSocket->close();
        }
        {         
            std::unique_lock<std::mutex> lock(mMutex);
            mShutdownSignal.wait(lock, [this]()->bool {
                return !mWaitingForEvents;
            } );
        }
        mManager->waitForTasksToComplete();
        mEventHandler->deleteMarkedOverlaps();
        mOverlaps.clear();
        //finish clean up
        closesocket(mSocket);
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
                    auto ioverlap = static_cast<IOverlap*>(overlaps[idx].lpOverlapped);
                    ioverlap->handleIOCompletion(overlaps[idx].dwNumberOfBytesTransferred);
                }
            }
        }
    }   
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mWaitingForEvents = false;
    }
    mShutdownSignal.notify_all();
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

        //if socket error, need to see if it was just pending
        //completed sends will be reported via i/o completion
        if(SOCKET_ERROR == WSASend(sckt->socket(), &overlap->mWsaBuffer, 1, &overlap->mBytes, overlap->mFlags, overlap, 0))
        {
            int lastError = WSAGetLastError();
            if(WSA_IO_PENDING != lastError)
            {
                mEventHandler->reportError(std::string("Error sending data") + std::to_string(lastError));
                mEventHandler->markForDeletion(*overlap);
            }
        }
    }
}

}
}
}
}
