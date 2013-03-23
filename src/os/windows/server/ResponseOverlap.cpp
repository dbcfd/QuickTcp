#include "os/windows/server/ResponseOverlap.h"
#include "os/windows/server/IEventHandler.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ResponseOverlap::ResponseOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler) 
    : IOverlap(sckt, evHandler, evHandler->responseBufferSize())
{

}

//------------------------------------------------------------------------------
ResponseOverlap::~ResponseOverlap()
{
    
}

//------------------------------------------------------------------------------
void ResponseOverlap::handleIOCompletion(const size_t nbBytes)
{
    if(hasOpenEvent())
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
        closeEvent();
    }
}

}
}
}
}
