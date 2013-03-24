#include "os/windows/client/IOverlap.h"

#include "utilities/ByteStream.h"

#include <assert.h>
#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace client {

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, size_t bufferSize) 
    : mSocket(sckt), mBytes(0), mFlags(0), mHasClosedEvent(false)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        std::stringstream sstr;
        sstr << "WSACreateEvent";
        throw(std::runtime_error(sstr.str()));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = std::shared_ptr<char>(new char[bufferSize]);
    memset(mBuffer.get(), 0, bufferSize);
    mWsaBuffer.buf = mBuffer.get();
    mWsaBuffer.len = bufferSize;
}

//------------------------------------------------------------------------------
IOverlap::IOverlap(std::shared_ptr<Socket> sckt, std::shared_ptr<utilities::ByteStream> stream) 
    : mSocket(sckt), mBytes(0), mFlags(0), mHasClosedEvent(false)
{
    if (WSA_INVALID_EVENT == (hEvent = WSACreateEvent()))
    {
        throw(std::runtime_error("WSACreateEvent"));
    }
    SecureZeroMemory(&mWsaBuffer, sizeof(WSABUF));
    mBuffer = std::shared_ptr<char>(new char[stream->size()]);
    memcpy(mBuffer.get(), stream->buffer(), stream->size());
    mWsaBuffer.buf = mBuffer.get();
    mWsaBuffer.len = stream->size();
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
