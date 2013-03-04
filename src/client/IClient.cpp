#include "client/IClient.h"

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
IClient::IClient(const ServerInfo& info, std::shared_ptr<IListener> listener)
    : mInfo(info), mListener(listener)
{

}

//------------------------------------------------------------------------------
void IClient::sendDataToListener(std::shared_ptr<utilities::ByteStream> stream)
{
    mListener->receive(stream);
}

}
}