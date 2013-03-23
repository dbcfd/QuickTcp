#include "os/windows/server/SendOverlap.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/Socket.h"

#include "utilities/ByteStream.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler,
        std::shared_ptr<utilities::ByteStream> stream) : IOverlap(sckt, evHandler, stream), mExpectedSize(stream->size())
{

}

//------------------------------------------------------------------------------
SendOverlap::~SendOverlap()
{

}

//------------------------------------------------------------------------------
void SendOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(hasOpenEvent())
    {
        mBytes += nbBytes;
        mFlags = 0;
        DWORD bytesSent = 0;
        if(WSAGetOverlappedResult(mSocket->socket(), this, &bytesSent, FALSE, &mFlags))
        {
            mBytes += bytesSent;
            completeSend();
        }
        else
        {
            //i/o wasn't complete, see if it was due to error or buffer fulle
            int err = WSAGetLastError();
            if(WSA_IO_INCOMPLETE == err)
            {
                mBytes += nbBytes;
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
    if(mExpectedSize != mBytes)
    {
        mEventHandler->reportError("Failed to send all bytes");
    }
    closeEvent();
}

}
}
}
}
