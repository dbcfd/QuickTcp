#pragma once

#include "os/windows/client/Platform.h"
#include "os/windows/client/Winsock2.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace os {
namespace windows {
namespace client {

class Socket;
class IEventHandler;

//------------------------------------------------------------------------------
struct IOverlap : public WSAOVERLAPPED {
    IOverlap(std::shared_ptr<Socket> socket, 
        const size_t bufferSize,
        std::shared_ptr<IEventHandler> handler);
    IOverlap(std::shared_ptr<Socket> socket, 
        std::shared_ptr<utilities::ByteStream> stream,
        std::shared_ptr<IEventHandler> handler);
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
    std::shared_ptr<utilities::ByteStream> mStream;
    std::shared_ptr<IEventHandler> mEventHandler;
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
