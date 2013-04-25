#include "quicktcp/os/windows/server/IOverlap.h"
#include "quicktcp/os/windows/server/IEventHandler.h"
#include "quicktcp/os/windows/server/Socket.h"

#include "quicktcp/utilities/ByteStream.h"

#include <assert.h>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt, const size_t bufferSize) 
    : mEventHandler(evHandler), mSocket(sckt), mBytes(0), mFlags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = new char[bufferSize];
    memset(mBuffer, 0, sizeof(char) * bufferSize);
    mWsaBuffer.buf = mBuffer;
    mWsaBuffer.len = (ULONG)bufferSize;
}

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<IEventHandler> evHandler, std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream) 
    : mEventHandler(evHandler), mSocket(sckt), mBytes(0), mFlags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = new char[stream->size()];
    memcpy(mBuffer, stream->buffer(), stream->size());
    mWsaBuffer.buf = mBuffer;
    mWsaBuffer.len = (ULONG)stream->size();
}

//------------------------------------------------------------------------------
IOverlap::~IOverlap()
{
    delete[] mBuffer;
}

//------------------------------------------------------------------------------
SOCKET IOverlap::winsocket() const
{
    return mSocket->socket();
}

//------------------------------------------------------------------------------
void IOverlap::closeEvent()
{
    if(hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(hEvent);
    }
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
bool IOverlap::getOverlappedResult()
{
    auto ret = WSAGetOverlappedResult(winsocket(), this, &mBytes, FALSE, &mFlags);
    ResetEvent(hEvent);
    return (ret == TRUE);
}

//------------------------------------------------------------------------------
void IOverlap::transferBufferToStream()
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
std::shared_ptr<utilities::ByteStream> IOverlap::transferStream()
{
    auto ret = mStream;
    mStream.reset();
    return ret;
}

}
}
}
}
