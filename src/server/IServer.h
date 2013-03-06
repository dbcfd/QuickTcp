#pragma once

#include "server/Platform.h"
#include "server/ServerInfo.h"

#include <atomic>
#include <functional>
#include <future>
#include <memory>

namespace async_cpp {
namespace async {
class AsyncResult;
}
}

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class IServerConnection;

/**
 * Server interface. Platform specific implementations will implement this interface, allowing
 * common access to the platform servers
 */
class SERVER_API IServer {
public:
    IServer(const ServerInfo& info);

    virtual void shutdown() = 0;
    virtual void waitForEvents() = 0;

protected:
    ServerInfo mInfo;
};

//inline implementations
//------------------------------------------------------------------------------

}
}