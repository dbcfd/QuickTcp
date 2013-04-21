#include "quicktcp/os/windows/server/SendCompleter.h"
#include "quicktcp/os/windows/server/Overlap.h"
#include "quicktcp/os/windows/server/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendCompleter::SendCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler,
        const size_t expectedSize) : ICompleter(sckt, evHandler), mExpectedSize(expectedSize)
{

}

//------------------------------------------------------------------------------
SendCompleter::~SendCompleter()
{

}

//------------------------------------------------------------------------------
void SendCompleter::handleIOCompletion(Overlap& holder, const size_t nbBytes)
{
    mTotalBytes = 0;
    if(holder.getOverlappedResult())
    {
        mTotalBytes += holder.bytes();
        completeSend(holder);
    }
    else
    {
        //i/o wasn't complete, see if it was due to error or buffer fulle
        int err = WSAGetLastError();
        if(WSA_IO_INCOMPLETE == err)
        {
            mTotalBytes += holder.bytes();
        }
        else
        {
            completeSend(holder);
        }
    }    
}

//------------------------------------------------------------------------------
void SendCompleter::completeSend(Overlap& holder)
{
    if(mExpectedSize != mTotalBytes)
    {
        holder.reportError("Failed to send all bytes");
    }
    mReadyForDeletion = true;
}

}
}
}
}
