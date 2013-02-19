#include "server/ServerInfo.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const unsigned int port, const unsigned int maxConnections) : mPort(port), mMaxConnections(maxConnections)
{

}

}
}