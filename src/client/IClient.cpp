#include "client/IClient.h"

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
IClient::IClient(const ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize)
    : mInfo(info), mAuthentication(authentication), mBufferSize(bufferSize)
{

}

}
}