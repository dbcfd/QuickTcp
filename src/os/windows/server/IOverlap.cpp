#include "os/windows/server/IOverlap.h"

#include "utilities/ByteStream.h"

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
IOverlap::IOverlap() : bytes(0), flags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        std::stringstream sstr;
        sstr << "WSACreateEvent";
        throw(std::runtime_error(sstr.str()));
    }
    SecureZeroMemory(&wsaBuffer, sizeof(WSABUF));
}

//------------------------------------------------------------------------------
IOverlap::~IOverlap()
{
    WSACloseEvent(hEvent);
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> IOverlap::transferStream()
{
    auto ret = stream;
    stream.reset();
    return ret;
}


}
}
}
}
