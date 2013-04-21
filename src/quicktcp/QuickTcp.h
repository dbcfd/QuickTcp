#pragma once

#include "quicktcp/Platform.h"

#include <functional>
#include <memory>

namespace async_cpp {
namespace async {
class AsyncResult;
}
namespace workers {
class IManager;
}
}

namespace quicktcp {

namespace client {
class IClient;
class ServerInfo;
}

namespace server {
class IResponder;
class IServer;
class ServerInfo;
}

namespace utilities {
class ByteStream;
}

extern QUICKTCP_API std::shared_ptr<client::IClient> CreateClient(const client::ServerInfo& info, 
                                                                  std::shared_ptr<utilities::ByteStream> authentication, 
                                                                  const size_t bufferSize,
                                                                  std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processStreamFunc);

extern QUICKTCP_API std::shared_ptr<server::IServer> CreateServer(const quicktcp::server::ServerInfo& info, 
                                                                  std::shared_ptr<async_cpp::workers::IManager> mgr, 
                                                                  std::shared_ptr<server::IResponder> responder);

}
