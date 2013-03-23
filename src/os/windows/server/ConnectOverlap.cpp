#include "os/windows/server/ConnectOverlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ReceiveOverlap.h"
#include "os/windows/server/ResponseOverlap.h"
#include "os/windows/server/Socket.h"

#include <minwinbase.h>
#include <iostream>
#include <sstream>

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
        int err = WSAGetLastError();
        throw(std::runtime_error("Io completion port error"));
    }
}

//------------------------------------------------------------------------------
ConnectOverlap::~ConnectOverlap()
{
    
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(hasOpenEvent())
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
                //handle the connection
                handleConnection();
            }
        }
        else
        {
            //i/o wasn't complete, see if it was due to error or buffer fulle
            int err = WSAGetLastError();
            if(WSA_IO_INCOMPLETE != err)
            {
                mEventHandler->reportError("Incomplete I/O Error when waiting for connection");
            }
            else
            {
                transferBufferToStream(mBytes);
            }
        }
    }
}

//------------------------------------------------------------------------------
void ConnectOverlap::reset()
{
    mPendingDisconnect = true;
    mSocket->disconnect(this);
    mEventHandler->connectionClosed();
    mReceiver = nullptr;
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleConnection()
{
    BOOL bOptVal = TRUE;
    int bOptLen = sizeof(BOOL);
    //update the socket context based on our server
    setsockopt(mSocket->socket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &bOptVal, bOptLen);

    mReceiver = new ReceiveOverlap(mSocket, mEventHandler, std::bind(&ConnectOverlap::reset, this));

    mEventHandler->authenticateConnection(transferStream(), mReceiver);
}

}
}
}
}
