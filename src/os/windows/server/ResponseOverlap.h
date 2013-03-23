#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

#include "async/AsyncResult.h"

#include <functional>

namespace quicktcp {

namespace server {
class IResponder;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
class ServerConnection;

//------------------------------------------------------------------------------
struct ResponseOverlap : public IOverlap {
    ResponseOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler);
    virtual ~ResponseOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);

    async_cpp::async::AsyncResult mResult;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
