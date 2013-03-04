#include "os/windows/server/IOverlap.h"

#include "utilities/ByteStream.h"

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
IOverlap::IOverlap(bool cnct) : isConnect(cnct)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        std::stringstream sstr;
        sstr << "WSACreateEvent";
        throw(std::runtime_error(sstr.str()));
    }
}

//------------------------------------------------------------------------------
IOverlap::~IOverlap()
{
    WSACloseEvent(hEvent);
    hEvent = WSA_INVALID_EVENT;
}

}
}
}
}
