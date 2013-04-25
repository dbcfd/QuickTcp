#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/Winsock2.h"

#include <memory>
#include <vector>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
class Socket;

//------------------------------------------------------------------------------
struct IOverlap : public WSAOVERLAPPED {
    IOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt, const size_t bufferSize);    
    IOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream);
    virtual ~IOverlap();

    virtual void handleIOCompletion(const size_t nbBytes) = 0;
    virtual void shutdown() = 0;

    SOCKET winsocket() const;

protected:
    void transferBufferToStream();
    std::shared_ptr<utilities::ByteStream> transferStream();
    bool getOverlappedResult();
    void closeEvent();

    WSABUF mWsaBuffer;
    DWORD mFlags;
    DWORD mBytes;
    char *mBuffer;
    std::shared_ptr<utilities::ByteStream> mStream;
    std::shared_ptr<Socket> mSocket;
    std::shared_ptr<IEventHandler> mEventHandler;
};

//Inline Implementations
//------------------------------------------------------------------------------

}
}
}
}
