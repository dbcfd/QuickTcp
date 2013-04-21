#include "os/windows/server/ResponseCompleter.h"
#include "os/windows/server/IEventHandler.h"
#include "os/windows/server/Overlap.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ResponseCompleter::ResponseCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler) 
    : ICompleter(sckt, evHandler), mReadyForDeletion(false)
{

}

//------------------------------------------------------------------------------
ResponseCompleter::~ResponseCompleter()
{
    
}

//------------------------------------------------------------------------------
void ResponseCompleter::handleIOCompletion(Overlap& holder, const size_t nbBytes)
{
    if(mResult.wasError())
    {
        holder.reportError(mResult.error());
    }
    else
    {
        auto response = std::static_pointer_cast<utilities::ByteStream>(mResult.result());
        mEventHandler->sendResponse(mSocket, response);
    }
    mReadyForDeletion = true;
}

}
}
}
}
