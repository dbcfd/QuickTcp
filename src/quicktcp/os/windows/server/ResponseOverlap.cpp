#include "quicktcp/os/windows/server/ResponseOverlap.h"
#include "quicktcp/os/windows/server/IEventHandler.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ResponseOverlap::ResponseOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt) 
    : IOverlap(evHandler, sckt, evHandler->responseBufferSize()), mShutdown(false)
{

}

//------------------------------------------------------------------------------
ResponseOverlap::~ResponseOverlap()
{
    
}

//------------------------------------------------------------------------------
void ResponseOverlap::shutdown()
{
    mShutdown = true;
    closeEvent();
}

//------------------------------------------------------------------------------
void ResponseOverlap::setResult(async_cpp::async::AsyncResult&& result)
{
    if(!mShutdown)
    {
        mResult = result;
        mEventHandler->postCompletion(this);
    }
}

//------------------------------------------------------------------------------
void ResponseOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(mShutdown)
    {
        mOnFinish();
    }
    else
    {
        if(mResult.wasError())
        {
            mEventHandler->reportError(mResult.error());
        }
        else
        {
            auto response = std::static_pointer_cast<utilities::ByteStream>(mResult.result());
            mEventHandler->sendResponse(mSocket, response);
        }
        shutdown();
    }
}

}
}
}
}
