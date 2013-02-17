#pragma once

#include "server/interface/IServerConnection.h"

#include "server/windows/Overlap.h"
#include "server/windows/Winsock2.h"

namespace quicktcp {
namespace server {
namespace windows {

class Socket;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class SERVER_WINDOWS_API ServerConnection: public iface::IServerConnection, public Overlap<ServerConnection>
{
public:
    typedef std::function<void(HANDLE)> ConnectionClosed;
	/**
	 * Utilizes the socket created through an asynchronous accept (AcceptEx), to maintain a connection
	 * between a server and a client
	 */
    ServerConnection(const Socket& socket, 
        const std::string& identifier, 
        workers::WorkerPool* pool,
        ConnectionClosed ccFunc);
	virtual ~ServerConnection();

	inline const SOCKET getSocket() const;

    virtual void close();

private:
    virtual bool blockingSend(const utilities::ByteStream& data);
    virtual utilities::ByteStream blockingReceive();

	SOCKET mSocket; //socket
	DWORD mBytes;//bytes received or sent
	char mBuffer[MAX_BUFFER_SIZE];//storage buffer
	WSABUF mDataBuffer;//Winsock2 specific buffer for send/recv
    ConnectionClosed mConnectionClosed;
};

const SOCKET ServerConnection::getSocket() const
{
    return mSocket;
}

}
}
}
