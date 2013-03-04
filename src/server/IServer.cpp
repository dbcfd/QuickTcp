#include "server/IServer.h"
#include "server/IServerConnection.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
IServer::IServer(const ServerInfo& info)
    : mInfo(info)
{
    
}

}
}