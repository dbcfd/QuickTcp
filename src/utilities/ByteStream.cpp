#include "Utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

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

//------------------------------------------------------------------------------
void ByteStream::append(std::shared_ptr<ByteStream> other)
{
    auto newSize = mSize + other->size();
    auto oldBuffer = mBuffer;
    mBuffer = malloc(sizeof(char) * newSize);
    auto bufPos = memcpy(mBuffer, oldBuffer, mSize);
    free(oldBuffer);
    memcpy(bufPos, other->buffer(), other->size());
    mSize = newSize;
}

}
}