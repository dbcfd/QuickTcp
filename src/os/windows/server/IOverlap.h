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
    IOverlap(bool isConnect);
    virtual ~IOverlap();

    bool isConnect;
    WSABUF wsaBuffer;
    DWORD bytesRead;
    std::shared_ptr<utilities::ByteStream> stream;
};

}
}
}
}
