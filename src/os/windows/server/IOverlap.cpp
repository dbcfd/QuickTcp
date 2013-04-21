#include "os/windows/server/IOverlap.h"

#include "utilities/ByteStream.h"

#include <assert.h>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, std::shared_ptr<IEventHandler> eventHandler, size_t bufferSize) 
    : mSocket(sckt), mEventHandler(eventHandler), mBytes(0), mFlags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = std::shared_ptr<char>(new char[bufferSize]);
    memset(mBuffer.get(), 0, bufferSize);
    mWsaBuffer.buf = mBuffer.get();
    mWsaBuffer.len = (ULONG)bufferSize;
}

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, std::shared_ptr<IEventHandler> eventHandler, std::shared_ptr<utilities::ByteStream> stream) 
    : mSocket(sckt), mEventHandler(eventHandler), mBytes(0), mFlags(0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = std::shared_ptr<char>(new char[stream->size()]);
    memcpy(mBuffer.get(), stream->buffer(), stream->size());
    mWsaBuffer.buf = mBuffer.get();
    mWsaBuffer.len = (ULONG)stream->size();
}

//------------------------------------------------------------------------------
IOverlap::~IOverlap()
{
    if(hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(hEvent);
    }
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
void IOverlap::transferBufferToStream(const size_t nbBytes)
{
    if(nbBytes > 0)
    {
        std::shared_ptr<utilities::ByteStream> transferred(new utilities::ByteStream((void*)mWsaBuffer.buf, nbBytes));
        if(nullptr == mStream)
        {
            mStream = std::shared_ptr<utilities::ByteStream>(transferred);
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
