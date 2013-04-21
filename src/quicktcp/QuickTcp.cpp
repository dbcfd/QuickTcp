#include "quicktcp/QuickTcp.h"

#ifdef WIN32
#include "quicktcp/os/windows/client/Client.h"
#include "quicktcp/os/windows/server/Server.h"
#else
#include "quicktcp/os/posix/client/Client.h"
#include "quicktcp/os/posix/server/Server.h"
#endif

namespace quicktcp {

QUICKTCP_API std::shared_ptr<client::IClient> CreateClient(const client::ServerInfo& info, 
                                                           std::shared_ptr<utilities::ByteStream> authentication, 
                                                           const size_t bufferSize)
{
#ifdef WIN32
    return std::make_shared<os::windows::client::Client>(info, authentication, bufferSize);
#endif
}

QUICKTCP_API std::shared_ptr<server::IServer> CreateServer(const quicktcp::server::ServerInfo& info, 
                                                           std::shared_ptr<async_cpp::workers::IManager> mgr, 
                                                           std::shared_ptr<server::IResponder> responder)
{
#ifdef WIN32
    return std::make_shared<os::windows::server::Server>(info, mgr, responder);
#endif
}

}
