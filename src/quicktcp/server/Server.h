#pragma once

#include "quicktcp/server/Platform.h"
#include "quicktcp/server/ServerInfo.h"

#include <atomic>
#include <memory>

namespace boost {
namespace asio {
class io_service;
}
}

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class IResponder;

/**
 * Implementation of a tcp server relying on boost::asio for tcp networking activities
 */
class SERVER_API Server {
public:
    Server(std::shared_ptr<boost::asio::io_service> ioService, 
        const ServerInfo& info, 
        std::shared_ptr<IResponder> responder);
    ~Server();

    void shutdown();
    void waitForEvents();

protected:
    class TcpConnection;
    class TcpServer;

    std::atomic_bool mRunning;
    std::unique_ptr<TcpServer> mServer;
    std::shared_ptr<boost::asio::io_service> mService;
};

//inline implementations
//------------------------------------------------------------------------------

}
}