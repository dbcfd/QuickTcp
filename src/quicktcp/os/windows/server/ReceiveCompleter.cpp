#include "quicktcp/os/windows/server/ReceiveCompleter.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/ResponseCompleter.h"
#include "quicktcp/os/windows/server/Socket.h"
#include "quicktcp/os/windows/server/Overlap.h"

#include "quicktcp/utilities/ByteStream.h"

#include <minwinbase.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ReceiveCompleter::ReceiveCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler, 
        std::function<void(void)> onDisconnect) 
    : ICompleter(sckt, evHandler), mOnDisconnect(onDisconnect), mReadyForDeletion(false)
{

}

//------------------------------------------------------------------------------
ReceiveCompleter::~ReceiveCompleter()
{
    
}

//------------------------------------------------------------------------------
void ReceiveCompleter::handleIOCompletion(Overlap& holder, const size_t nbBytes)
{
    if(holder.getOverlappedResult())
    {
        //receive when connected, no bytes means disconnect
        if(holder.bytes() == 0)
        {
            disconnect();
        }
        else
        {
            //handle read
            prepareToRespond(holder);
            prepareToReceive(holder);
        }
    }
    else
    {
        //i/o wasn't complete, see if it was due to error or buffer fulle
        int err = WSAGetLastError();
        if(WSA_IO_INCOMPLETE != err)
        {
            mEventHandler->reportError(std::string("Incomplete I/O error when trying to receive: ") + std::to_string(err));
            disconnect();
        }
    }
}

//------------------------------------------------------------------------------
void ReceiveCompleter::prepareToRespond(Overlap& holder)
{
    auto completer = std::make_shared<ResponseCompleter>(mSocket, mEventHandler);
    auto overlap = new Overlap(completer, mEventHandler->responseBufferSize());
    holder.transferBufferToStream();
    mEventHandler->createResponse(holder.transferStream(), overlap);
}

//------------------------------------------------------------------------------
void ReceiveCompleter::prepareToReceive(Overlap& holder)
{
    if(mSocket->isValid())
    {
        /**
         * We need to queue an asynchronous receive, but since we're queue'ing
         * a receive, there exists the possibility that there is data ready to be
         * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
         */
        while(SOCKET_ERROR != holder.queueReceive())
        {
            //if no bytes, connection is shutting down
            if(0 == holder.bytes())
            {
                disconnect();
            }
            else
            {
                prepareToRespond(holder);
            }
        }

        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            mEventHandler->reportError(std::string("Error prepping client for receive: ") + std::to_string(lastError));
            disconnect();
        }
    }
}

//------------------------------------------------------------------------------
void ReceiveCompleter::disconnect()
{
    mOnDisconnect();
    mReadyForDeletion = true;
}

}
}
}
}
