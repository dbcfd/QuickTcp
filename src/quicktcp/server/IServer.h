#pragma once

#include "server/Platform.h"
#include "server/ServerInfo.h"

#include <memory>

namespace async_cpp {
namespace workers {
class IManager;
}
}

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class IResponder;

/**
 * Server interface. Platform specific implementations will implement this interface, allowing
 * common access to the platform servers
 */
class SERVER_API IServer {
public:
    IServer(const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<async_cpp::workers::IManager> mgr, 
        std::shared_ptr<IResponder> responder);

    virtual void shutdown() = 0;
    virtual void waitForEvents() = 0;

protected:
    ServerInfo mInfo;
    std::shared_ptr<async_cpp::workers::IManager> mManager;
    std::shared_ptr<quicktcp::server::IResponder> mResponder;
};

//inline implementations
//------------------------------------------------------------------------------

}
}