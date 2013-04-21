#include "quicktcp/os/windows/server/Overlap.h"
#include "quicktcp/os/windows/server/ICompleter.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

#include <assert.h>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
Overlap::Overlap(std::shared_ptr<ICompleter> completer, const size_t bufferSize) 
    : mCompleter(completer), mBytes(0), mFlags(0), mBuffer(bufferSize, 0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mWsaBuffer.buf = &mBuffer[0];
    mWsaBuffer.len = (ULONG)bufferSize;
}

//------------------------------------------------------------------------------
Overlap::Overlap(std::shared_ptr<ICompleter> completer, std::shared_ptr<utilities::ByteStream> stream) 
    : mCompleter(completer), mBytes(0), mFlags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer.resize(stream->size());
    mBuffer.assign((const char*)stream->buffer(), (const char*)stream->buffer() + stream->size());
    mWsaBuffer.buf = &mBuffer[0];
    mWsaBuffer.len = (ULONG)stream->size();
}

//------------------------------------------------------------------------------
Overlap::~Overlap()
{
    if(hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(hEvent);
    }
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
bool Overlap::queueAcceptEx(LPFN_ACCEPTEX pfnAcceptEx, SOCKET serverSocket)
{
    auto acceptExResult = pfnAcceptEx(serverSocket, winsocket(), &mBuffer[0],
        0, //mInfo.bufferSize() - addrSize,
        sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &mBytes,
        this);
    return (acceptExResult == TRUE);
}

//------------------------------------------------------------------------------
bool Overlap::getOverlappedResult()
{
    auto ret = WSAGetOverlappedResult(winsocket(), this, &mBytes, FALSE, &mFlags);
    ResetEvent(hEvent);
    return (ret == TRUE);
}

//------------------------------------------------------------------------------
int Overlap::queueReceive()
{
    return WSARecv(winsocket(), &mWsaBuffer, 1, &mBytes, &mFlags, this, 0);
}

//------------------------------------------------------------------------------
int Overlap::queueSend()
{
    return WSASend(winsocket(), &mWsaBuffer, 1, &mBytes, mFlags, this, 0);
}

//------------------------------------------------------------------------------
void Overlap::handleIOCompletion(const size_t nbBytes)
{
    if(mCompleter)
    {
        mCompleter->handleIOCompletion(*this, nbBytes);
    }
}

//------------------------------------------------------------------------------
void Overlap::shutdown()
{
    mCompleter.reset();
    if(hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(hEvent);
    }
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
bool Overlap::readyForDeletion()
{
    bool ret = (nullptr == mCompleter);
    if(mCompleter->readyForDeletion())
    {
        shutdown();
    }
    return ret;
}

//------------------------------------------------------------------------------
void Overlap::transferBufferToStream()
{
    if(mBytes > 0)
    {
        auto transferred = std::make_shared<utilities::ByteStream>((void*)mWsaBuffer.buf, mBytes);
        if(nullptr == mStream)
        {
            mStream = transferred;
        }
        else
        {
            mStream->append(transferred);
        }
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> Overlap::transferStream()
{
    auto ret = mStream;
    mStream.reset();
    return ret;
}

//------------------------------------------------------------------------------
void Overlap::reportError(const std::string& error)
{
    if(mCompleter)
    {
        mCompleter->mEventHandler->reportError(error);
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<Socket> Overlap::socket() const
{
    std::shared_ptr<Socket> ret;
    if(mCompleter)
    {
        ret = mCompleter->mSocket;
    }
    return ret;
}

//------------------------------------------------------------------------------
SOCKET Overlap::winsocket() const
{
    SOCKET ret = INVALID_SOCKET;
    if(mCompleter)
    {
        ret = mCompleter->mSocket->socket();
    }
    return ret;
}

}
}
}
}
