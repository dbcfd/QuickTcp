#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/IOverlap.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
class Socket;

//------------------------------------------------------------------------------
class SendOverlap : public IOverlap
{
public:
    SendOverlap(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler,
        std::shared_ptr<utilities::ByteStream> stream);
    ~SendOverlap();

    virtual void handleIOCompletion(const size_t nbBytes);
    virtual std::string isA() { return "Send"; }

    void completeSend();

    size_t mExpectedSize;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
