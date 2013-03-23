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
class IEventHandler;

//------------------------------------------------------------------------------
struct IOverlap : public WSAOVERLAPPED {
    IOverlap(std::shared_ptr<Socket> socket, 
        std::shared_ptr<IEventHandler> eventHandler, 
        const size_t bufferSize);
    IOverlap(std::shared_ptr<Socket> socket, 
        std::shared_ptr<IEventHandler> eventHandler, 
        std::shared_ptr<utilities::ByteStream> stream);
    virtual ~IOverlap();

    virtual void handleIOCompletion(const size_t nbBytes) = 0;

    void closeEvent();
    void transferBufferToStream(const size_t nbBytes);
    std::shared_ptr<utilities::ByteStream> transferStream();
    inline bool requiresDeletion() const;
    inline bool hasOpenEvent() const;

    WSABUF mWsaBuffer;
    DWORD mBytes;
    DWORD mFlags;
    std::shared_ptr<Socket> mSocket;
    std::shared_ptr<IEventHandler> mEventHandler;
    std::shared_ptr<utilities::ByteStream> mStream;
    std::shared_ptr<char> mBuffer;
    bool mHasClosedEvent;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool IOverlap::requiresDeletion() const
{
    return mHasClosedEvent;
}

//------------------------------------------------------------------------------
bool IOverlap::hasOpenEvent() const
{
    return hEvent != WSA_INVALID_EVENT;
}

}
}
}
}
