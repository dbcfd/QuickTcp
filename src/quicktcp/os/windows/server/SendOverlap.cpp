#include "quicktcp/os/windows/server/SendOverlap.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(std::shared_ptr<IEventHandler> evHandler, 
                             std::shared_ptr<Socket> sckt, 
                             std::shared_ptr<utilities::ByteStream> stream) 
                             : IOverlap(evHandler, sckt, stream), mExpectedSize(stream->size()), mShutdown(false)
{

}

//------------------------------------------------------------------------------
SendOverlap::~SendOverlap()
{

}

//------------------------------------------------------------------------------
int SendOverlap::queueSend()
{
    return WSASend(mSocket->socket(), &mWsaBuffer, 1, &mBytes, mFlags, this, 0);
}

//------------------------------------------------------------------------------
void SendOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(mShutdown)
    {
        delete this;
    }
    else
    {
        mTotalBytes = 0;
        if(getOverlappedResult())
        {
            mTotalBytes += mBytes;
            completeSend();
        }
        else
        {
            //i/o wasn't complete, see if it was due to error or buffer fulle
            int err = WSAGetLastError();
            if(WSA_IO_INCOMPLETE == err)
            {
                mTotalBytes += mBytes;
            }
            else
            {
                completeSend();
            }
        }    
    }
}

//------------------------------------------------------------------------------
void SendOverlap::completeSend()
{
    if(mExpectedSize != mTotalBytes)
    {
        mEventHandler->reportError("Failed to send all bytes");
    }
    shutdown();
}

//------------------------------------------------------------------------------
void SendOverlap::shutdown()
{
    mShutdown = true;
    mEventHandler->postCompletion(this);
}

}
}
}
}
