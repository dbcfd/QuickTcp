#pragma once

#include "os/windows/Server/Platform.h"
#include "os/windows/Server/Winsock2.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

/**
 * Wrapper around the underlying Winsock2 socket implementation, which maintains
 * a network connection to some other device.
 */
class WINDOWSSERVER_API Socket
{
public:
    enum SocketState
    {
        OP_WAITING = 0, OP_READ, OP_WRITE
    };
    /**
     * Create a socket, using next available socket, setting appropriate attribution.
     */
    Socket();

    ~Socket();

    void disconnect(WSAOVERLAPPED& overlap);

    inline void close();
    inline SOCKET socket() const;

private:
    SOCKET mSocket;
};

//Inline Implementations
//------------------------------------------------------------------------------
void Socket::close()
{
    closesocket(mSocket);
}

//------------------------------------------------------------------------------
SOCKET Socket::socket() const
{
    return mSocket;
}

}
}
}
}
