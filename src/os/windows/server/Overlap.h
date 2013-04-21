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

class ICompleter;
class Socket;

//------------------------------------------------------------------------------
struct Overlap : public WSAOVERLAPPED {
    Overlap(std::shared_ptr<ICompleter> completer, const size_t bufferSize);    
    Overlap(std::shared_ptr<ICompleter> completer, std::shared_ptr<utilities::ByteStream> stream);
    ~Overlap();

    void handleIOCompletion(const size_t nbBytes);
    bool readyForDeletion();
    bool queueAcceptEx(LPFN_ACCEPTEX pfnAcceptEx, SOCKET serverSocket);
    bool getOverlappedResult();
    int queueReceive();
    int queueSend();

    void transferBufferToStream();
    std::shared_ptr<utilities::ByteStream> transferStream();
    void reportError(const std::string& error);
    void shutdown();
    std::shared_ptr<Socket> socket() const;
    SOCKET winsocket() const;

    inline std::shared_ptr<ICompleter> completer() const;
    inline DWORD bytes() const;

private:
    WSABUF mWsaBuffer;
    DWORD mFlags;
    DWORD mBytes;
    std::shared_ptr<char> mBuffer;
    std::shared_ptr<utilities::ByteStream> mStream;
    std::shared_ptr<ICompleter> mCompleter;
};

//Inline Implementations
//------------------------------------------------------------------------------
std::shared_ptr<ICompleter> Overlap::completer() const
{
    return mCompleter;
}

//------------------------------------------------------------------------------
DWORD Overlap::bytes() const
{
    return mBytes;
}

}
}
}
}
