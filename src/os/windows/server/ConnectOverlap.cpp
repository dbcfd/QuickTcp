#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ReceiveOverlap.h"
#include "os/windows/server/ResponseOverlap.h"
#include "os/windows/server/Socket.h"

#include <minwinbase.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ConnectOverlap::ConnectOverlap(std::shared_ptr<Socket> sckt,
                               std::shared_ptr<IEventHandler> evHandler,
                               HANDLE mainIOCP) 
    : IOverlap(sckt, evHandler, evHandler->connectBufferSize()), mPendingDisconnect(false), mReceiver(nullptr)
{
    if(mainIOCP != CreateIoCompletionPort((HANDLE)mSocket->socket(), mainIOCP, (ULONG_PTR)this, 0))
    {
        auto error = std::string("CreateIoCompletionPort Error: ") + std::to_string(WSAGetLastError());
        throw(std::runtime_error(error));
    }
}

//------------------------------------------------------------------------------
ConnectOverlap::~ConnectOverlap()
{
    
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleIOCompletion(const size_t nbBytes)
{
    mFlags = 0;
    if(WSAGetOverlappedResult(mSocket->socket(), this, &mBytes, FALSE, &mFlags))
    {
        ResetEvent(hEvent);
        if(mPendingDisconnect)
        {
            mPendingDisconnect = false;
            mEventHandler->queueAccept(*this);
        }
        else
        {
            //handle the connection, only if the socket is valid, otherwise we're closing down
            if(mSocket->isValid())
            {
                handleConnection();
            }
        }
    }
    else
    {
        //i/o wasn't complete, see if it was due to error or buffer fulle
        int err = WSAGetLastError();
        if(WSA_IO_INCOMPLETE != err)
        {
            mEventHandler->reportError(std::string("Incomplete I/O Error when waiting for connection") + std::to_string(err));
        }
        else
        {
            transferBufferToStream(mBytes);
        }
    }
}

//------------------------------------------------------------------------------
void ConnectOverlap::reset()
{
    if(mReceiver)
    {
        mPendingDisconnect = true;
        mSocket->disconnect(this);
        mEventHandler->connectionClosed();
    }
    mReceiver = nullptr;
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleConnection()
{
    BOOL opt = TRUE;
    setsockopt(mSocket->socket(), SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(BOOL));

    mReceiver = new ReceiveOverlap(mSocket, mEventHandler, std::bind(&ConnectOverlap::reset, this));

    mEventHandler->authenticateConnection(transferStream(), *mReceiver);
}

//------------------------------------------------------------------------------
void ConnectOverlap::waitForDisconnect()
{
    if(mPendingDisconnect)
    {
        WSAGetOverlappedResult(mSocket->socket(), this, &mBytes, FALSE, &mFlags);
    }
}

}
}
}
}
