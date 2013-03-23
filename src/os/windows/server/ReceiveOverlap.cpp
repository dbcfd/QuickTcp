#include "os/windows/server/ReceiveOverlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ResponseOverlap.h"
#include "os/windows/server/Socket.h"

#include "utilities/ByteStream.h"

#include <minwinbase.h>
#include <iostream>
#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ReceiveOverlap::ReceiveOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler, 
        std::function<void(void)> onDisconnect) 
    : IOverlap(sckt, evHandler, evHandler->receiveBufferSize()), mOnDisconnect(onDisconnect), mIsAuthenticated(false)
{

}

//------------------------------------------------------------------------------
ReceiveOverlap::~ReceiveOverlap()
{
    
}

//------------------------------------------------------------------------------
void ReceiveOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(hasOpenEvent())
    {
        if(!mIsAuthenticated)
        {
            mIsAuthenticated = true;
            prepareToReceive();
        }
        else
        {
            mFlags = 0;
            if(WSAGetOverlappedResult(mSocket->socket(), this, &mBytes, FALSE, &mFlags))
            {
                ResetEvent(hEvent);
                //receive when connected, no bytes means disconnect
                if(mBytes == 0)
                {
                    disconnect();
                }
                else
                {
                    //handle read
                    transferBufferToStream(mBytes);
                    mEventHandler->createResponse(transferStream(), new ResponseOverlap(mSocket, mEventHandler));
                    prepareToReceive();
                }
            }
            else
            {
                //i/o wasn't complete, see if it was due to error or buffer fulle
                int err = WSAGetLastError();
                if(WSA_IO_INCOMPLETE != err)
                {
                    mEventHandler->reportError("Incomplete I/O error when trying to receive");
                    disconnect();
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void ReceiveOverlap::prepareToReceive()
{
    mFlags = 0;
    if(mSocket->isValid())
    {
        /**
         * We need to queue an asynchronous receive, but since we're queue'ing
         * a receive, there exists the possibility that there is data ready to be
         * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
         */
        while(SOCKET_ERROR != WSARecv(mSocket->socket(), &mWsaBuffer, 1, &mBytes, &mFlags, this, 0))
        {
            //if no bytes, connection is shutting down
            if(0 == mBytes)
            {
                disconnect();
            }
            else
            {
                transferBufferToStream(mBytes);
                mEventHandler->createResponse(transferStream(), new ResponseOverlap(mSocket, mEventHandler));
            }
        }

        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            std::stringstream sstr;
            sstr << "Error prepping client socket for receive" << WSAGetLastError();
            mEventHandler->reportError(sstr.str());
            disconnect();
        }
    }
}

//------------------------------------------------------------------------------
void ReceiveOverlap::disconnect()
{
    mOnDisconnect();
    closeEvent();
}

}
}
}
}
