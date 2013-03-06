#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/Winsock2.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace os {
namespace windows {
namespace server {

class Socket;

//------------------------------------------------------------------------------
struct IOverlap : public WSAOVERLAPPED {
    IOverlap();
    virtual ~IOverlap();

    virtual bool handleIOCompletion(SOCKET sckt, const size_t nbBytes) = 0;

    WSABUF wsaBuffer;
    DWORD bytes;
    std::shared_ptr<utilities::ByteStream> stream;
};

}
}
}
}
