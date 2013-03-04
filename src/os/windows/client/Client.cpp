#include "os/windows/client/Client.h"

#include "utilities/ByteStream.h"

#include <async/AsyncResult.h>

#include <workers/IManager.h>
#include <workers/BasicTask.h>

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
struct Client::ReceiveOverlap : public WSAOVERLAPPED {
    ReceiveOverlap() 
    {
        SecureZeroMemory((PVOID)this, sizeof (WSAOVERLAPPED));
        if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
        {
            std::stringstream sstr;
            sstr << "WSACreateEvent";
            throw(std::runtime_error(sstr.str()));
        }
    }

    ~ReceiveOverlap()
    {
        WSACloseEvent(hEvent);
        hEvent = WSA_INVALID_EVENT;
    }

    std::shared_ptr<utilities::ByteStream> stream;
};

//------------------------------------------------------------------------------
Client::Client(std::shared_ptr<async_cpp::workers::IManager> mgr, 
        const quicktcp::client::ServerInfo& info,
        std::shared_ptr<IListener> listener,
        const size_t allocationSize) 
    : quicktcp::client::IClient(info, listener), mManager(mgr),
    mAllocatedStorage(new char[allocationSize]), mAllocationSize(allocationSize), mConnected(false), mSendsOutstanding(0)
{
   //startup winsock
    WSAData wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(0 != iResult)
    {
        std::stringstream sstr;
        sstr << "WSAStartup Error " << iResult;
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    mReceiveOverlap = std::shared_ptr<ReceiveOverlap>(new ReceiveOverlap());

    connect(info);
}

//------------------------------------------------------------------------------
void Client::connect(const quicktcp::client::ServerInfo& info) {
    //define where we're connecting to
    struct addrinfo* results = nullptr, *addrptr, hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream sstr;
    sstr << info.port();

    //get the address information for the host
    int iResult = getaddrinfo(info.address().c_str(), sstr.str().c_str(), &hints, &results);

    if(0 != iResult)
    {
        std::stringstream sstr;
        sstr << "getaddrinfo failed: " << iResult;
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    if(nullptr == results)
    {
        std::stringstream sstr;
        sstr << "no results for server " << info.address();
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    addrptr = results;

    //multiple addresses may be returned work through them all
    while(addrptr)
    {
        Socket socket;

        WSABUF dataBuffer;
        dataBuffer.buf = 0;
        dataBuffer.len = 0;

        //attempt to connect to the address information
        if(SOCKET_ERROR
                == WSAConnect(socket.getSocket(), addrptr->ai_addr, (int) addrptr->ai_addrlen,
                        &dataBuffer, 0, 0, 0))
        {
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

    if(0 == mSocket.getSocket() || INVALID_SOCKET == mSocket.getSocket())
    {
        std::stringstream sstr;
        sstr << "Failed to create client connection" << std::endl;
        throw(std::runtime_error(sstr.str()));
    }
    mBuffer.buf = mAllocatedStorage.get();
    mBuffer.len = mAllocationSize;

    mConnected = true;

    //queue our receive, we are now ready to go
    prepareClientToReceiveData();
}

//------------------------------------------------------------------------------
void Client::prepareClientToReceiveData()
{
    DWORD flags = 0;
    DWORD nbBytesReceived = 0;

    /**
     * We need to queue an asynchronous receive, but since we're queue'ing
     * a receive, there exists the possibility that there is data ready to be
     * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
     */
    while(SOCKET_ERROR != WSARecv(mSocket.getSocket(), 
        &mBuffer, 1, &nbBytesReceived, &flags, mReceiveOverlap.get(), 0))
    {
        std::shared_ptr<utilities::ByteStream> stream(new utilities::ByteStream((void*)mBuffer.buf, (size_t)nbBytesReceived));
        sendDataToListener(stream);
    }

    int lastError = WSAGetLastError();

    if(WSA_IO_PENDING != lastError)
    {
        mConnected = false;
        std::stringstream sstr;
        sstr << "Error prepping client socket for receive" << WSAGetLastError();
        throw(std::runtime_error(sstr.str()));
    }

    if(WSAECONNRESET == lastError) //server has shutdown, client no longer needed
    {
        mConnected = false;
        mListener->serverDisconnected();
    }
}

//------------------------------------------------------------------------------
Client::~Client()
{
    disconnect();
    mSocket = INVALID_SOCKET;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mSendsOutstandingSignal.wait(lock, [this]()->bool{
        size_t outstanding = mSendsOutstanding;
        return (outstanding == 0);
    } );
}

//------------------------------------------------------------------------------
void Client::disconnect()
{
    if(mConnected)
    {
        mConnected = false;

        ::send(mSocket.getSocket(), nullptr, 0, 0);

        /**
         * Close out our socket to sending and receiving. This doesn't actually send
         * or receive disconnection signals.
         */
        WSASendDisconnect(mSocket.getSocket(), 0);
        WSARecvDisconnect(mSocket.getSocket(), 0);

        mSocket.closeSocket();
    }
}

//------------------------------------------------------------------------------
struct Client::SendOverlap : public WSAOVERLAPPED {
    SendOverlap(Client& cl, std::shared_ptr<utilities::ByteStream> str) : client(cl), stream(str)
    {
        SecureZeroMemory((PVOID)this, sizeof (WSAOVERLAPPED));
        if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
        {
            std::stringstream sstr;
            sstr << "WSACreateEvent";
            throw(std::runtime_error(sstr.str()));
        }

        buffer.len = stream->size();
        buffer.buf = (CHAR*) stream->buffer();
    }

    ~SendOverlap()
    {
        WSACloseEvent(hEvent);
        hEvent = WSA_INVALID_EVENT;
    }

    Client& client;
    WSABUF buffer;
    std::promise<async_cpp::async::AsyncResult> promise;
    std::shared_ptr<utilities::ByteStream> stream;
};

//------------------------------------------------------------------------------
void CALLBACK ClientSendCompletion (
  IN DWORD dwError, 
  IN DWORD cbTransferred, 
  IN LPWSAOVERLAPPED lpOverlapped, 
  IN DWORD dwFlags
)
{
    auto so = static_cast<Client::SendOverlap*>(lpOverlapped);
    if(nullptr == so->stream)
    {
        so->stream = so->client.stream(cbTransferred);
    }
    else
    {
        so->stream->append(so->client.stream(cbTransferred));
    }
    //no socket error, means no pending i/o
    if(SOCKET_ERROR != dwError)
    {
        so->promise.set_value(async_cpp::async::AsyncResult(so->client.stream(cbTransferred)));
        so->client.completeSend();
        delete so;
    }
}

//------------------------------------------------------------------------------
std::future<async_cpp::async::AsyncResult> Client::send(std::shared_ptr<utilities::ByteStream> byteStream)
{
    if(!mConnected)
    {
        std::promise<async_cpp::async::AsyncResult> promise;
        promise.set_value(async_cpp::async::AsyncResult("Client disconnected"));
        return promise.get_future();
    }

    DWORD flags = 0;
    auto sendOverlap = new SendOverlap(*this, byteStream);

    /**
     * Perform an asynchronous send, with no callback. This uses the overlapped
     * structure and its event handle to determine when the send is complete.
     */
    mSendsOutstanding++;
    int iResult = WSASend(mSocket.getSocket(), &sendOverlap->buffer, 1, 0, flags, sendOverlap, &ClientSendCompletion);

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
            throw(std::runtime_error("not io pending"));
        }
    }
    /**
     * If the send completed immediately, we need to use the overlapped structure
     * to determine how many bytes were sent.
     */
    else if(0 == iResult)
    {
        DWORD nbBytes = 0;
        if(WSAGetOverlappedResult(mSocket.getSocket(), sendOverlap, &nbBytes, FALSE, 0))
        {
            completeSend();
            sendOverlap->promise.set_value(async_cpp::async::AsyncResult(stream(nbBytes)));
        }
    }

    return sendOverlap->promise.get_future();
}

//------------------------------------------------------------------------------
void Client::completeSend()
{
    mSendsOutstanding--;
    mSendsOutstandingSignal.notify_all();
}

//------------------------------------------------------------------------------
void Client::waitForEvents()
{
    WSAEVENT events[] = {mReceiveOverlap->hEvent};
    while(mConnected)
    {
        if(WSA_WAIT_FAILED != WSAWaitForMultipleEvents(1, events, true, WSA_INFINITE, true))
        {
            WSAResetEvent(mReceiveOverlap->hEvent);
            DWORD cbTransferred = 0;
            DWORD flags = 0;
            if(WSAGetOverlappedResult(mSocket.getSocket(), mReceiveOverlap.get(), &cbTransferred, FALSE, &flags))
            {
                if(cbTransferred == 0)
                {
                    disconnect();
                }
                else 
                {
                    if(nullptr == mReceiveOverlap->stream)
                    {
                        mReceiveOverlap->stream = stream(cbTransferred);
                    }
                    else
                    {
                        mReceiveOverlap->stream->append(stream(cbTransferred));
                    }
                    //if not io incomplete, can send our data
                    if(WSA_IO_INCOMPLETE != flags && nullptr != mReceiveOverlap->stream)
                    {
                        mManager->run(std::shared_ptr<async_cpp::workers::BasicTask>(new async_cpp::workers::BasicTask(std::bind(
                            [](Client& client, std::shared_ptr<utilities::ByteStream> stream)->void {
                                client.sendDataToListener(stream);  
                            }, std::ref(*this), mReceiveOverlap->stream)
                        ) ) );
                        mReceiveOverlap->stream.reset();
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> Client::stream(size_t nbBytes) const
{
    return std::shared_ptr<utilities::ByteStream>(new utilities::ByteStream(mBuffer.buf, nbBytes));
}

}
}
}
}