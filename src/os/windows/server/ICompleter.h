#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/Winsock2.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace os {
namespace windows {
namespace server {

class Socket;
class IEventHandler;
struct Overlap;

//------------------------------------------------------------------------------
class ICompleter {
public:
    ICompleter(std::shared_ptr<Socket> socket, 
        std::shared_ptr<IEventHandler> eventHandler);
    virtual ~ICompleter();

    virtual void handleIOCompletion(Overlap& holder, const size_t nbBytes) = 0;
    virtual bool readyForDeletion() const = 0;

    std::shared_ptr<Socket> mSocket;
    std::shared_ptr<IEventHandler> mEventHandler;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
