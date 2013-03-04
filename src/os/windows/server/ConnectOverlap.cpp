#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/Socket.h"
#include "os/windows/server/ServerConnection.h"

#include "utilities/ByteStream.h"

#include <minwinbase.h>
#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ConnectOverlap::ConnectOverlap(std::shared_ptr<Socket> sckt, const size_t bufferSize) : IOverlap(true), socket(sckt), isReset(false)
{
    buffer = std::shared_ptr<char>(new char[bufferSize]);
    wsaBuffer.buf = &(buffer.get()[0]);
    wsaBuffer.len = 0;
}

//------------------------------------------------------------------------------
ConnectOverlap::~ConnectOverlap()
{
    
}

//------------------------------------------------------------------------------
void ConnectOverlap::reset()
{
    isReset = true;
    socket->disconnect(*this);
    currentConnection.reset();
}

//------------------------------------------------------------------------------
VOID CALLBACK IoCompletionCallback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    if(0 == dwErrorCode)
    {
        auto overlap = static_cast<ConnectOverlap*>(lpOverlapped);
        if(WSAGetOverlappedResult(overlap->socket->socket(), overlap, &overlap->bytesRead, FALSE, 0))
        {
            overlap->transferBufferToStream(overlap->bytesRead);
            overlap->currentConnection->processResponse(overlap->stream);
        }
        else
        {
            if(WSA_IO_INCOMPLETE == WSAGetLastError())
            {
                overlap->transferBufferToStream(overlap->bytesRead);
            }
        }
    }
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleConnection(HANDLE mainIOCP, std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<quicktcp::server::IResponder> responder)
{
    iocp = CreateIoCompletionPort((HANDLE)socket->socket(), mainIOCP, (ULONG_PTR)socket->socket(), 0);
    ULONG flags = 0;
    LPOVERLAPPED_COMPLETION_ROUTINE func = &IoCompletionCallback;
    BindIoCompletionCallback((HANDLE)socket->socket(), func, flags);

    BOOL bOptVal = TRUE;
    int bOptLen = sizeof(BOOL);
    //update the socket context based on our server
    setsockopt(socket->socket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &bOptVal, bOptLen);
    currentConnection = std::shared_ptr<ServerConnection>(new ServerConnection(*this, evHandler, responder));

    /**
    * When a connection is formed, we will be receiving information from that client at
    * some point in time. We queue a receive that will call back to a method that
    * can handle the connection information.
    */
    currentConnection->prepareToReceive();
}

//------------------------------------------------------------------------------
void ConnectOverlap::transferBufferToStream(const size_t nbBytes)
{
    std::shared_ptr<utilities::ByteStream> transferred(new utilities::ByteStream((void*)wsaBuffer.buf, nbBytes));
    if(nullptr == stream)
    {
        stream = std::shared_ptr<utilities::ByteStream>(transferred);
    }
    else
    {
        stream->append(transferred);
    }
}

}
}
}
}
