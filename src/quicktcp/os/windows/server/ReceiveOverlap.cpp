#include "quicktcp/os/windows/server/ReceiveOverlap.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/ResponseOverlap.h"
#include "quicktcp/os/windows/server/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

#include <minwinbase.h>
#include <string>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ReceiveOverlap::ReceiveOverlap(std::shared_ptr<IEventHandler> evHandler, 
                                   std::shared_ptr<Socket> sckt, 
                                   std::function<void(void)> onFinish) 
    : IOverlap(evHandler, sckt, evHandler->receiveBufferSize()), mOnFinish(onFinish), mState(State::Valid)
{

}

//------------------------------------------------------------------------------
ReceiveOverlap::~ReceiveOverlap()
{
    
}

//------------------------------------------------------------------------------
void ReceiveOverlap::handleIOCompletion( const size_t nbBytes)
{
    switch(mState)
    {
    case State::Valid:
        if(getOverlappedResult())
        {
            //receive when connected, no bytes means disconnect
            if(0 == mBytes)
            {
                shutdown();
            }
            else
            {
                //handle read
                prepareToRespond();
                prepareToReceive();
            }
        }
        else
        {
            //i/o wasn't complete, see if it was due to error or buffer fulle
            int err = WSAGetLastError();
            if(WSA_IO_INCOMPLETE != err)
            {
                mEventHandler->reportError(std::string("Incomplete I/O error when trying to receive: ") + std::to_string(err));
                shutdown();
            }
        }
        break;
    case State::ClosingResponses:
        {
            std::unique_lock<std::mutex> lock(mResponseMutex);
            if(mOutstandingResponses.empty())
            {
                mState = State::ReadyForFinish;
            }
            mEventHandler->postCompletion(this);
        }
        break;
    default:
        mOnFinish();
        break;
    }
}

//------------------------------------------------------------------------------
void ReceiveOverlap::prepareToRespond()
{
    auto finishFunc = [this](std::shared_ptr<ResponseOverlap> overlap)->void {
        std::unique_lock<std::mutex> lock(mResponseMutex);
        mOutstandingResponses.erase(overlap);
        if(mState == State::ClosingResponses) mEventHandler->postCompletion(this);
    };
    transferBufferToStream();
    auto overlap = std::make_shared<ResponseOverlap>(mEventHandler, mSocket);
    overlap->setFinishFunction(std::bind(finishFunc, overlap));
    mEventHandler->createResponse(transferStream(), overlap);
}

//------------------------------------------------------------------------------
int ReceiveOverlap::queueReceive()
{
    return WSARecv(mSocket->socket(), &mWsaBuffer, 1, &mBytes, &mFlags, this, 0);
}

//------------------------------------------------------------------------------
void ReceiveOverlap::prepareToReceive()
{
    if(mSocket->isValid())
    {
        /**
         * We need to queue an asynchronous receive, but since we're queue'ing
         * a receive, there exists the possibility that there is data ready to be
         * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
         */
        while(SOCKET_ERROR != queueReceive())
        {
            //if no bytes, connection is shutting down
            if(0 == mBytes)
            {
                shutdown();
            }
            else
            {
                prepareToRespond();
            }
        }

        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            mEventHandler->reportError(std::string("Error prepping client for receive: ") + std::to_string(lastError));
            shutdown();
        }
    }
}

//------------------------------------------------------------------------------
void ReceiveOverlap::shutdown()
{
    {
        std::unique_lock<std::mutex> lock(mResponseMutex);
        if(!mOutstandingResponses.empty())
        {
            mState = State::ClosingResponses;
            for(auto responder : mOutstandingResponses)
            {
                responder->shutdown();
            }
        }
        else
        {
            mState = State::ReadyForFinish;
            mEventHandler->postCompletion(this);
        }
    }
}

}
}
}
}
