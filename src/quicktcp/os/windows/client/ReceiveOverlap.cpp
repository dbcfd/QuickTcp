#include "quicktcp/os/windows/client/ReceiveOverlap.h"
#include "quicktcp/os/windows/client/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
ReceiveOverlap::ReceiveOverlap(std::shared_ptr<Socket> sckt, 
                         const size_t recvBufferSize,
                         std::promise<async_cpp::async::AsyncResult>& promise,
                         std::shared_ptr<IEventHandler> handler) 
                         : IOverlap(sckt, recvBufferSize, handler), mPromise(std::move(promise))
{

}

//------------------------------------------------------------------------------
ReceiveOverlap::~ReceiveOverlap()
{

}

//------------------------------------------------------------------------------
void ReceiveOverlap::handleIOCompletion(const size_t nbBytes)
{
    //still processing
    if(hasOpenEvent())
    {
        mFlags = 0;
        if(WSAGetOverlappedResult(mSocket->socket(), this, &mBytes, FALSE, &mFlags))
        {
            ResetEvent(hEvent);
            //receive when connected, no bytes means disconnect
            if(mBytes == 0)
            {
                shutdown("Server sent no bytes");
            }
            else
            {
                //handle read
                transferBufferToStream(mBytes);
                mPromise.set_value(async_cpp::async::AsyncResult(transferStream()));
                closeEvent();
            }
        }
        else
        {
            //i/o wasn't complete, see if it was due to error or buffer fulle
            int err = WSAGetLastError();
            if(WSA_IO_INCOMPLETE != err)
            {
                shutdown(std::string("Incomplete I/O error when trying to receive: ") + std::to_string(err));
            }
        } 
    }
}

//------------------------------------------------------------------------------
void ReceiveOverlap::shutdown(const std::string& message)
{
    mPromise.set_value(async_cpp::async::AsyncResult(message));
    closeEvent();
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
        while(SOCKET_ERROR != WSARecv(mSocket->socket(), &mWsaBuffer, 1, &mBytes, &mFlags, this, 0) && hasOpenEvent())
        {
            //if no bytes, connection is shutting down
            if(0 == mBytes)
            {
                shutdown("Connection to server was closed prior to receiving response");
            }
            else
            {
                transferBufferToStream(mBytes);
                mPromise.set_value(async_cpp::async::AsyncResult(transferStream()));
                closeEvent();
            }
        }

        int lastError = WSAGetLastError();

        if(hasOpenEvent() && WSA_IO_PENDING != lastError)
        {
            shutdown(std::string("Error prepping client socket for receive") + std::to_string(lastError));
        }
    }
}

}
}
}
}
