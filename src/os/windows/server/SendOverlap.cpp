#include "os/windows/server/SendOverlap.h"

#include "utilities/ByteStream.h"

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(SOCKET sckt, std::shared_ptr<utilities::ByteStream> str) : IOverlap(), socket(sckt)
{
    sendComplete = false;
    stream = str;
    wsaBuffer.buf = (char*)str->buffer();
    wsaBuffer.len = str->size();
}

//------------------------------------------------------------------------------
SendOverlap::~SendOverlap()
{

}

//------------------------------------------------------------------------------
bool SendOverlap::handleIOCompletion(SOCKET sckt, const size_t nbBytes)
{
    flags = 0;
    bool ret = true;
    DWORD bytesSent = 0;
    if(WSAGetOverlappedResult(socket, this, &bytesSent, FALSE, &flags))
    {
        bytes += bytesSent;
        completeSend(bytes);
    }
    else
    {
        //i/o wasn't complete, see if it was due to error or buffer fulle
        int err = WSAGetLastError();
        if(WSA_IO_INCOMPLETE == err)
        {
            ret = false;
            bytes += nbBytes;
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
void SendOverlap::completeSend(const size_t nbBytes)
{
    bool sendAlreadyComplete = sendComplete.exchange(true);
    if(!sendAlreadyComplete)
    {
        if(stream->size() != nbBytes)
        {
            promise.set_value(async_cpp::async::AsyncResult("Failed to send all bytes"));
        }
        else
        {
            promise.set_value(async_cpp::async::AsyncResult());
        }
    }
}

}
}
}
}
