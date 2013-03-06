#include "os/windows/server/SendOverlap.h"

#include "utilities/ByteStream.h"

#include <sstream>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

//------------------------------------------------------------------------------
SendOverlap::SendOverlap(SOCKET sckt, std::shared_ptr<utilities::ByteStream> str) : IOverlap(), socket(sckt), flags(0)
{
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
    bool ret = false;
    DWORD bytesSent = 0;
    if(WSAGetOverlappedResult(socket, this, &bytesSent, FALSE, &flags))
    {
        bytes += bytesSent;
        completeSend(bytes);
        ret = true;
    }
    else
    {
        //i/o wasn't complete, see if it was due to error or buffer fulle
        int err = WSAGetLastError();
        if(WSA_IO_INCOMPLETE != err)
        {
            ret = true;
        }
        else
        {
            bytes += nbBytes;
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
void SendOverlap::completeSend(const size_t nbBytes)
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
