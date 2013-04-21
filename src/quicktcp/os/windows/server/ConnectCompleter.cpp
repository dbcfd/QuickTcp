#include "os/windows/server/ConnectCompleter.h"
#include "os/windows/server/Overlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/ReceiveCompleter.h"
#include "os/windows/server/Socket.h"

#include <minwinbase.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ConnectCompleter::ConnectCompleter(std::shared_ptr<Socket> sckt,
                               std::shared_ptr<IEventHandler> evHandler) 
    : ICompleter(sckt, evHandler), mPendingDisconnect(false), mReadyForDeletion(false)
{
    
}

//------------------------------------------------------------------------------
ConnectCompleter::~ConnectCompleter()
{
    
}

//------------------------------------------------------------------------------
void ConnectCompleter::handleIOCompletion(Overlap& holder, const size_t nbBytes)
{
    if(holder.getOverlappedResult())
    {
        if(mPendingDisconnect)
        {
            mPendingDisconnect = false;
            mEventHandler->queueAccept(std::static_pointer_cast<ConnectCompleter>(holder.completer()));
        }
        else
        {
            //handle the connection, only if the socket is valid, otherwise we're closing down
            if(mSocket->isValid())
            {
                handleConnection(holder);
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
            holder.transferBufferToStream();
        }
    }
}

//------------------------------------------------------------------------------
void ConnectCompleter::reset(Overlap* holder)
{
    if(mReceiver)
    {
        mPendingDisconnect = true;
        mReadyForDeletion = mSocket->disconnect(holder);
        mEventHandler->connectionClosed();
    }
    mReceiver = nullptr;
}

//------------------------------------------------------------------------------
void ConnectCompleter::handleConnection(Overlap& holder)
{
    BOOL opt = TRUE;
    setsockopt(mSocket->socket(), SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(BOOL));

    auto completer = holder.completer();
    auto resetFunc = [this, completer]()->void {
        auto nextHolder = new Overlap(completer, mEventHandler->connectBufferSize());
        reset(nextHolder);
    };

    mReceiver = std::make_shared<ReceiveCompleter>(mSocket, mEventHandler, resetFunc);
    auto recvHolder = new Overlap(mReceiver, mEventHandler->responseBufferSize());

    mEventHandler->authenticateConnection(holder.transferStream(), recvHolder);
}

//------------------------------------------------------------------------------
void ConnectCompleter::waitForDisconnect(Overlap& holder)
{
    if(mPendingDisconnect)
    {
        holder.getOverlappedResult();
    }
}

}
}
}
}
