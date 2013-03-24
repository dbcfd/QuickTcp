#include "os/windows/client/Client.h"
#include "os/windows/client/ReceiveOverlap.h"
#include "os/windows/client/SendOverlap.h"
#include "os/windows/client/IEventHandler.h"

#include "async/AsyncResult.h"

#include "utilities/ByteStream.h"

#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
class Client::EventHandler : public IEventHandler
{
public:
    EventHandler(Client& client) : mClient(client)
    {

    }

    virtual void createReceiveOverlap(std::shared_ptr<Socket> socket, std::promise<async_cpp::async::AsyncResult>& promise)
    {
        auto overlap = new ReceiveOverlap(socket, mClient.bufferSize(), promise, mClient.mEventHandler);
        overlap->prepareToReceive();
    }

    virtual async_cpp::async::AsyncResult processStream(std::shared_ptr<utilities::ByteStream> stream)
    {
        return mClient.processStreamFunction()(stream);
    }

    Client& mClient;
};

//------------------------------------------------------------------------------
Client::Client(const quicktcp::client::ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize,
        std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processStreamFunc) 
    : quicktcp::client::IClient(info, authentication, bufferSize, processStreamFunc), mIsRunning(false)
{
   //startup winsock
    WSAData wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(0 != iResult)
    {
        auto error = std::string("WSA Startup Error: ") + std::to_string(iResult);
        WSACleanup();
        throw(std::runtime_error(error));
    }

    mEventHandler = std::shared_ptr<EventHandler>(new EventHandler(*this));

    connect();
}

//------------------------------------------------------------------------------
void Client::connect() 
{
    mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    //define where we're connecting to
    struct addrinfo* results = nullptr, *addrptr, hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    auto port = std::to_string(mInfo.port());

    //get the address information for the host
    int iResult = getaddrinfo(mInfo.address().c_str(), port.c_str(), &hints, &results);

    if(0 != iResult)
    {
        auto error = std::string("getaddrinfo failed: ") + std::to_string(iResult);
        WSACleanup();
        throw(std::runtime_error(error));
    }

    if(nullptr == results)
    {
        auto error = std::string("no results for server ") + mInfo.address();
        WSACleanup();
        throw(std::runtime_error(error));
    }

    addrptr = results;

    std::shared_ptr<Socket> socket(new Socket());
    WSABUF dataBuffer;
    if(mAuthentication)
    {
        dataBuffer.buf = (char*)mAuthentication->buffer();
        dataBuffer.len = mAuthentication->size();
    }
    std::vector<int> errorsReturned;
    //multiple addresses may be returned work through them all
    while(addrptr)
    {
        //attempt to connect to the address information
        int result = SOCKET_ERROR;
        if(mAuthentication)
        {
            result = WSAConnect(socket->socket(), addrptr->ai_addr, (int) addrptr->ai_addrlen, &dataBuffer, 0, 0, 0);
        }
        else
        {
            result = WSAConnect(socket->socket(), addrptr->ai_addr, (int) addrptr->ai_addrlen, 0, 0, 0, 0);
        }
        if(SOCKET_ERROR == result)
        {
            errorsReturned.emplace_back(WSAGetLastError());
            addrptr = addrptr->ai_next;
        }
        else
        {
            //successfully connected, we found a server to use
            mSocket = socket;
            addrptr = nullptr;
        }
    }

    freeaddrinfo(results);
    results = nullptr;

    if(0 == mSocket->socket() || INVALID_SOCKET == mSocket->socket())
    {
        auto error = std::string("Failed to create client connection:");
        for(auto errCode : errorsReturned)
        {
            error += std::string(" ") + std::to_string(errCode);
        }
        throw(std::runtime_error(error));
    }

    //create IOCP on our socket
    CreateIoCompletionPort((HANDLE)mSocket->socket(), mIOCP, (ULONG_PTR)mSocket->socket(), 0);
    mIsRunning = true;
    //start listening
    mThread = std::thread([this]()->void {
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
        OVERLAPPED_ENTRY overlaps[10];
        ULONG overlapsReturned = 0;
        while(GetQueuedCompletionStatusEx(mIOCP, overlaps, sizeof(overlaps), &overlapsReturned, 10, TRUE))
        {
            for(auto idx = ULONG(0); idx < overlapsReturned; ++idx)
            {
                delete overlaps[idx].lpOverlapped;
            }
        }
    });
}

//------------------------------------------------------------------------------
Client::~Client()
{
    disconnect();
}

//------------------------------------------------------------------------------
void Client::disconnect()
{
    mIsRunning = false;
    if(mThread.joinable())
    {
        /**
            * Close out our socket to sending and receiving. This doesn't actually send
            * or receive disconnection signals.
            */
        WSASendDisconnect(mSocket->socket(), 0);
        WSARecvDisconnect(mSocket->socket(), 0);

        mSocket->close();

        PostQueuedCompletionStatus(mIOCP, 0, 0, 0);

        mThread.join();

        CloseHandle(mIOCP);
    }
}

//------------------------------------------------------------------------------
std::future<async_cpp::async::AsyncResult> Client::request(std::shared_ptr<utilities::ByteStream> byteStream)
{
    if(mSocket->socket() == INVALID_SOCKET)
    {
        std::promise<async_cpp::async::AsyncResult> promise;
        promise.set_value(async_cpp::async::AsyncResult("Client disconnected"));
        return promise.get_future();
    }

    DWORD flags = 0;
    auto sendOverlap = new SendOverlap(mSocket, byteStream, mEventHandler);

    /**
     * Perform an asynchronous send, with no callback. This uses the overlapped
     * structure and its event handle to determine when the send is complete.
     */
    int iResult = WSASend(mSocket->socket(), &sendOverlap->mWsaBuffer, 1, &sendOverlap->mWsaBuffer.len, flags, sendOverlap, 0);

    /**
     * Asynchronous send returns a SOCKET_ERROR if the send does not complete immediately.
     * Since this is asynchronous, it rarely completes immediately, so we then
     * need to check for WSA_IO_PENDING which shows that the send was successful, but
     * i/o is currently underway.
     */
    if(SOCKET_ERROR == iResult)
    {
        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            sendOverlap->shutdown(std::string("Error attempting send: ") + std::to_string(lastError));
        }
    }
    /**
     * If the send completed immediately, we need to use the overlapped structure
     * to determine how many bytes were sent.
     */
    else
    {
        DWORD nbBytes = 0;
        if(WSAGetOverlappedResult(mSocket->socket(), sendOverlap, &nbBytes, FALSE, 0))
        {
            sendOverlap->completeSend();
        }
    }

    return sendOverlap->mPromise.get_future();
}

}
}
}
}