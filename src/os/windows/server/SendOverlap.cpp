#include "os/windows/server/SendOverlap.h"

#include "utilities/ByteStream.h"

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(std::shared_ptr<utilities::ByteStream> str) : IOverlap(false)
{
    wsaBuffer.buf = (char*)str->buffer();
    wsaBuffer.len = str->size();
}

//------------------------------------------------------------------------------
SendOverlap::~SendOverlap()
{

}

}
}
}
}
