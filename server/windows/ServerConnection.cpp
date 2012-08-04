#ifdef WINDOWS
#include "server/windows/ServerConnection.h"
#include "server/windows/Socket.h"

namespace quicktcp {
namespace server {
namespace windows {

ServerConnection::ServerConnection(const Socket& socket, 
        const std::string& identifier, 
        workers::WorkerPool* pool,
        ServerConnection::ConnectionClosed ccFunc) :
Overlap<ServerConnection>(), iface::IServerConnection(identifier, pool), mSocket(socket.getSocket()), mBytes(0), mConnectionClosed(ccFunc)
{
    SecureZeroMemory(mBuffer, sizeof(mBuffer) / sizeof(mBuffer[0]));

    mDataBuffer.len = sizeof(mBuffer);
    mDataBuffer.buf = mBuffer;
    createOverlap(this);
}

ServerConnection::~ServerConnection()
{
    if(connected())
    {
        close();
    }
}

void ServerConnection::close()
{
    disconnect();
    DWORD nbBytes = 0;
    mConnectionClosed(getOverlap()->hEvent);
    //wake up any sockets trying to receive
    WSASetEvent(getOverlap()->hEvent);
    WSARecvDisconnect(mSocket, 0);
    WSASendDisconnect(mSocket, 0);
    closesocket(mSocket);
    mSocket = INVALID_SOCKET;
}

utilities::ByteStream ServerConnection::blockingReceive()
{
    DWORD flags = 0;
    DWORD nbBytes = 0;

    utilities::ByteStream ret;
    //make sure connection exists and recv has completed
    if(WSAGetOverlappedResult(mSocket, getOverlap(), &nbBytes, TRUE, &flags))
    {
        if(0 == nbBytes)
        {
            close();
        }
        else
        {
            ret = utilities::ByteStream(mBuffer, nbBytes);
        }
    }

    //queue the socket to receive again
    if(connected() && 0 != nbBytes)
    {
        receive();
    }
    return ret;
}

bool ServerConnection::blockingSend(const utilities::ByteStream& data)
{
    WSABUF dataBuffer;
    DWORD flags = 0;
    DWORD nbBytes = 0;
    dataBuffer.len = data.getSize();
    dataBuffer.buf = (CHAR*) data.getBuffer();

    WSAOVERLAPPED* overlap = getOverlap();

    bool ret = true;
    int rc = WSASend(mSocket, &dataBuffer, 1, &nbBytes, 0, overlap, nullptr);
    ret = ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()));

    if(ret && connected())
    {
        rc = WSAWaitForMultipleEvents(1, &(overlap->hEvent), TRUE, INFINITE, TRUE);
        ret = (rc == WSA_WAIT_FAILED);
    }

    if(ret && connected())
    {
        rc = WSAGetOverlappedResult(mSocket, overlap, &nbBytes, FALSE, &flags);
        ret = (rc == FALSE);
    }

    WSAResetEvent(overlap->hEvent);
    return ret;
}

}
}
}

#endif
