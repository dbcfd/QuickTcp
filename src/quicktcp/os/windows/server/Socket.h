#pragma once

#include "quicktcp/os/windows/Server/Platform.h"
#include "quicktcp/os/windows/Server/Winsock2.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

struct IOverlap;
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

    bool disconnect(IOverlap* overlap);

    inline void close();
    inline SOCKET socket() const;
    inline bool isValid() const;

private:
    SOCKET mSocket;
};

//Inline Implementations
//------------------------------------------------------------------------------
void Socket::close()
{
    closesocket(mSocket);
    mSocket = INVALID_SOCKET;
}

//------------------------------------------------------------------------------
SOCKET Socket::socket() const
{
    return mSocket;
}

//------------------------------------------------------------------------------
bool Socket::isValid() const
{
    return (mSocket != INVALID_SOCKET);
}

}
}
}
}
