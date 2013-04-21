#include "quicktcp/server/ServerInfo.h"

namespace quicktcp {
namespace server {

//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const unsigned int port, const size_t maxConnections, const size_t maxBacklog, const size_t bufferSize) 
    : mPort(port), mMaxConnections(maxConnections), mMaxBacklog(maxBacklog), mBufferSize(bufferSize)
{

}

}
}