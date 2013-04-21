#include "quicktcp/os/windows/client/IOverlap.h"

#include "quicktcp/utilities/ByteStream.h"

#include <assert.h>
#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, size_t bufferSize, std::shared_ptr<IEventHandler> handler) 
    : mSocket(sckt), mBytes(0), mFlags(0), mHasClosedEvent(false), mEventHandler(handler), mBuffer(bufferSize, 0)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        std::stringstream sstr;
        sstr << "WSACreateEvent";
        throw(std::runtime_error(sstr.str()));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mWsaBuffer.buf = &mBuffer[0];
    mWsaBuffer.len = (ULONG)bufferSize;
}

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream, std::shared_ptr<IEventHandler> handler) 
    : mSocket(sckt), mBytes(0), mFlags(0), mHasClosedEvent(false), mEventHandler(handler)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer.reserve(stream->size());
    mBuffer.assign((const char*)stream->buffer(), (const char*)stream->buffer() + stream->size());
    mWsaBuffer.buf = &mBuffer[0];
    mWsaBuffer.len = (ULONG)stream->size();
}

//------------------------------------------------------------------------------
IOverlap::~IOverlap()
{
    
}

//------------------------------------------------------------------------------
void IOverlap::closeEvent()
{
    mHasClosedEvent = true;
    WSACloseEvent(hEvent);
    hEvent = WSA_INVALID_EVENT;
}

//------------------------------------------------------------------------------
void IOverlap::transferBufferToStream(const size_t nbBytes)
{
    if(nbBytes > 0)
    {
        auto transferred = std::make_shared<utilities::ByteStream>((void*)mWsaBuffer.buf, nbBytes);
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
