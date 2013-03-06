#include "os/windows/server/ServerConnection.h"
#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/Socket.h"

#include "utilities/ByteStream.h"

#include <assert.h>
#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ServerConnection::ServerConnection(ConnectOverlap& overlap, 
        std::shared_ptr<IEventHandler> evHandler,
        std::shared_ptr<quicktcp::server::IResponder> responder) 
        : quicktcp::server::IServerConnection(responder), mOverlap(overlap), mEventHandler(evHandler), mFlags(0)
{
    assert(nullptr != evHandler);
}

//------------------------------------------------------------------------------
ServerConnection::~ServerConnection()
{
    disconnect();
}

//------------------------------------------------------------------------------
void ServerConnection::disconnect()
{
    mOverlap.reset();
}

//------------------------------------------------------------------------------
void ServerConnection::prepareToReceive()
{
    /**
     * We need to queue an asynchronous receive, but since we're queue'ing
     * a receive, there exists the possibility that there is data ready to be
     * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
     */
    while(SOCKET_ERROR != WSARecv(mOverlap.socket->socket(), &mOverlap.wsaBuffer, 1, &mOverlap.bytes, &mFlags, &mOverlap, 0))
    {
        mOverlap.transferBufferToStream(mOverlap.bytes);
        processResponse(mOverlap.stream);
    }

    int lastError = WSAGetLastError();

    if(WSA_IO_PENDING != lastError)
    {
        std::stringstream sstr;
        sstr << "Error prepping client socket for receive" << WSAGetLastError();
        throw(std::runtime_error(sstr.str()));
    }

    if(WSAECONNRESET == lastError) //client has shutdown, connection no longer needed
    {
        mOverlap.reset();
    }
}

//------------------------------------------------------------------------------
void ServerConnection::processResponse(std::shared_ptr<utilities::ByteStream> stream)
{
    mEventHandler->handleResponse(getResponse(stream));
}

}
}
}
}
