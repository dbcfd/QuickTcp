#include "quicktcp/QuickTcp.h"

#ifdef WIN32
#include "os/windows/client/Client.h"
#include "os/windows/server/Server.h"
#else
#include "os/posix/client/Client.h"
#include "os/posix/server/Server.h"
#endif

namespace quicktcp {

QUICKTCP_API std::shared_ptr<client::IClient> CreateClient(const client::ServerInfo& info, 
                                                           std::shared_ptr<utilities::ByteStream> authentication, 
                                                           const size_t bufferSize,
                                                           std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processStreamFunc)
{
#ifdef WIN32
    return std::shared_ptr<client::IClient>(new os::windows::client::Client(info, authentication, bufferSize, processStreamFunc));
#endif
}

QUICKTCP_API std::shared_ptr<server::IServer> CreateServer(const quicktcp::server::ServerInfo& info, 
                                                           std::shared_ptr<async_cpp::workers::IManager> mgr, 
                                                           std::shared_ptr<server::IResponder> responder)
{
#ifdef WIN32
    return std::shared_ptr<server::IServer>(new os::windows::server::Server(info, mgr, responder));
#endif
}

}
