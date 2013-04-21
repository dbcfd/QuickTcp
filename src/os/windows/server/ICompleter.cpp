#include "os/windows/server/ICompleter.h"

#include <assert.h>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
ICompleter::ICompleter(std::shared_ptr<Socket> sckt, std::shared_ptr<IEventHandler> eventHandler) 
    : mSocket(sckt), mEventHandler(eventHandler)
{

}

//------------------------------------------------------------------------------
ICompleter::~ICompleter()
{

}

}
}
}
}
