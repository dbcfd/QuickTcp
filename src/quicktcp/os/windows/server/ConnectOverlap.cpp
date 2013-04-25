#include "quicktcp/os/windows/server/ConnectOverlap.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/ReceiveOverlap.h"
#include "quicktcp/os/windows/server/Socket.h"

#include <minwinbase.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ConnectOverlap::ConnectOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt) 
    : IOverlap(evHandler, sckt, evHandler->connectBufferSize()), mPendingDisconnect(false), mShuttingDown(false)
{
    
}

//------------------------------------------------------------------------------
ConnectOverlap::~ConnectOverlap()
{
    
}

//------------------------------------------------------------------------------
void ConnectOverlap::prepareForClientConnection(SOCKET serverSocket)
{
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;

    size_t addrSize = 2*sizeof(SOCKADDR_STORAGE) + 32;

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR == WSAIoctl(serverSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &acceptex_guid, sizeof(acceptex_guid), &pfnAcceptEx, sizeof(pfnAcceptEx),
        &bytes, this, 0))
    {
        int lasterror = WSAGetLastError();
        mEventHandler->reportError("Failed to obtain AcceptEx() pointer");
        shutdown();
    }

    if(TRUE != pfnAcceptEx(serverSocket, mSocket->socket(), &mBuffer[0],
        0, //mInfo.bufferSize() - addrSize,
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &mBytes,
        this))
    {
        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            shutdown(); 
            mEventHandler->reportError("AcceptEx Error()");
        }
    }
    else
    {
        handleIOCompletion(bytes);
    }
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(mShuttingDown)
    {
        mReceiver.reset();
    }
    else
    {
        if(getOverlappedResult())
        {
            if(mPendingDisconnect)
            {
                mPendingDisconnect = false;
                mEventHandler->queueAccept(this);
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
                mShuttingDown = true;
            }
            else
            {
                transferBufferToStream();
            }
        }
    }
}

//------------------------------------------------------------------------------
void ConnectOverlap::shutdown()
{
    if(mReceiver)
    {
        mReceiver->shutdown();
    }
    else
    {
        disconnect();
    }
    mShuttingDown = true;
}

//------------------------------------------------------------------------------
void ConnectOverlap::disconnect()
{
    if(!mSocket->disconnect(this))
    {
        mEventHandler->reportError("Error disconnecting socket");
    }
    mEventHandler->connectionClosed();
}

//------------------------------------------------------------------------------
void ConnectOverlap::handleConnection()
{
    static auto finishFunc = [this]()->void {
        mReceiver.reset();
        if(mShuttingDown) 
        {
            mEventHandler->postCompletion(this);
        }
        else
        {
            disconnect();
        }
    };

    if(mEventHandler->authenticateConnection(transferStream()))
    {
        BOOL opt = TRUE;
        setsockopt(mSocket->socket(), SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(BOOL));

        mReceiver = std::make_shared<ReceiveOverlap>(mEventHandler, mSocket, finishFunc);

        mEventHandler->authenticateConnection(transferStream());
    }
    else
    {
        disconnect();
    }
}

}
}
}
}
