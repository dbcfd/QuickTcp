#pragma once

#include "server/windows/Platform.h"
#include "server/windows/Winsock2.h"

namespace quicktcp {
namespace server {
namespace windows {

/**
 * Wrapper around the underlying Winsock2 socket implementation, which maintains
 * a network connection to some other device.
 */
class SERVER_WINDOWS_API Socket
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
    /**
     * Provide a wrapper around an existing socket. This allows us to use wrapper functionality
     * on a socket that may have been created in another method (e.g. accept)
     */
    Socket(const SOCKET sckt);
    ~Socket();
    /**
     * Close the underlying Winsock2 socket
     */
    void closeSocket();

    SOCKET getSocket() const;

private:

    SOCKET mSocket;
};

}
}
}
