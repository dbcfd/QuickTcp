#include "Cache/ByteStream.h"

namespace markit {
namespace cache {

//------------------------------------------------------------------------------
ByteStream::ByteStream(const void* buffer, const size_t size) : mSize(size)
{
    mBuffer = malloc(sizeof(char) * size);
    memcpy(mBuffer, buffer, size);
}

//------------------------------------------------------------------------------
ByteStream::ByteStream(void* buffer, const size_t size, const bool takeOwnershipOfBuffer)
{
    if(takeOwnershipOfBuffer)
    {
        mBuffer = buffer;
    }
    else
    {
        mBuffer = malloc(sizeof(char) * size);
        memcpy(mBuffer, buffer, size);
    }
    mSize = size;
}

//------------------------------------------------------------------------------
ByteStream::~ByteStream()
{
    free(mBuffer);
}

}
}