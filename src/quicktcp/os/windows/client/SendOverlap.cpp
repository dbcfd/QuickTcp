#include "quicktcp/os/windows/client/SendOverlap.h"
#include "quicktcp/os/windows/client/IEventHandler.h"
#include "quicktcp/os/windows/client/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(std::shared_ptr<Socket> sckt, 
                         std::shared_ptr<utilities::ByteStream> stream,
                         std::shared_ptr<IEventHandler> handler) 
                         : IOverlap(sckt, stream, handler)
{

}

//------------------------------------------------------------------------------
SendOverlap::~SendOverlap()
{

}

//------------------------------------------------------------------------------
void SendOverlap::handleIOCompletion(const size_t nbBytes)
{
    //still processing
    if(hasOpenEvent())
    {
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
                mBytes += bytesSent;
            }
            else
            {
                shutdown(std::string("Error while sending: ") + std::to_string(err));
            }
        }    
    }
}

//------------------------------------------------------------------------------
void SendOverlap::completeSend()
{
    if(mWsaBuffer.len != mBytes)
    {
        mPromise.set_value(async_cpp::async::AsyncResult("Failed to send complete request"));
    }
    else
    {
        mEventHandler->createReceiveOverlap(mSocket, mPromise);
    }
    closeEvent();
}

//------------------------------------------------------------------------------
void SendOverlap::shutdown(const std::string& message)
{
    mPromise.set_value(async_cpp::async::AsyncResult(message));
    closeEvent();
}

}
}
}
}
