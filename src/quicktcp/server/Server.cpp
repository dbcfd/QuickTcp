#include "quicktcp/server/IServer.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
IServer::IServer(const quicktcp::server::ServerInfo& info, std::shared_ptr<async_cpp::workers::IManager> mgr, std::shared_ptr<IResponder> responder)
    : mInfo(info), mManager(mgr), mResponder(responder)
{
    
}

}
}