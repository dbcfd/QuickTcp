#include "os/windows/Server/Socket.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
Socket::Socket()
{
    mSocket = INVALID_SOCKET;
    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if(mSocket == INVALID_SOCKET)
    {
        throw(std::runtime_error("Socket Creation Error"));
    }

    //
    // Disable send buffering on the socket.  Setting SO_SNDBUF
    // to 0 causes winsock to stop buffering sends and perform
    // sends directly from our buffers, thereby save one memory copy.
    //
    // However, this does prevent the socket from ever filling the
    // send pipeline. This can lead to packets being sent that are
    // not full (i.e. the overhead of the IP and TCP headers is
    // great compared to the amount of data being carried).
    //
    // Disabling the send buffer has less serious repercussions
    // than disabling the receive buffer.
    //
    int nZero = 0;
    int nRet = setsockopt(mSocket, SOL_SOCKET, SO_SNDBUF, (char*) &nZero, sizeof(nZero));

    if(nRet == SOCKET_ERROR)
    {
        //TODO: log
    }
}

//------------------------------------------------------------------------------
Socket::~Socket()
{

}

//------------------------------------------------------------------------------
void Socket::disconnect(WSAOVERLAPPED& overlap)
{
    LPFN_DISCONNECTEX pfn;
    GUID guid = WSAID_DISCONNECTEX;
    DWORD bytes = 0;

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR == WSAIoctl(mSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &pfn, sizeof(pfn), &bytes, &overlap, nullptr))
    {
        throw(std::runtime_error("Failed to obtain DisconnectEx() pointer"));
    }

    DWORD flags = TF_REUSE_SOCKET;
    if(!(mSocket, &overlap, flags, 0))
    {
        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            close();
            throw(std::runtime_error("DisconnectEx Error()"));
        }
    }
}

}
}
}
}
