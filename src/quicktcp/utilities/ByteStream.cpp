#include "quicktcp/utilities/ByteStream.h"

#include <assert.h>

namespace quicktcp {
namespace utilities {

//------------------------------------------------------------------------------
ByteStream::ByteStream(const stream_data_t* buffer, const stream_size_t size) : mSize(size)
{
    assert(size);
    assert(buffer);
    mBuffer = new stream_data_t[size];
    memcpy(mBuffer, buffer, size);
}

//------------------------------------------------------------------------------
ByteStream::ByteStream(stream_data_t* buffer, const stream_size_t size, const bool takeOwnershipOfBuffer)
{
    assert(size);
    assert(buffer);

    if(takeOwnershipOfBuffer)
    {
        mBuffer = buffer;
    }
    else
    {
        mBuffer = new stream_data_t[size];
        memcpy(mBuffer, buffer, size);
    }
    mSize = size;
}

//------------------------------------------------------------------------------
ByteStream::~ByteStream()
{
    delete[] mBuffer;
}

//------------------------------------------------------------------------------
std::shared_ptr<ByteStream> ByteStream::append(std::shared_ptr<ByteStream> other) const
{
    assert(other);
    auto combinedSize = mSize + other->size();
    auto combinedBuffer = new stream_data_t[combinedSize];
    //copy original first into buffer
    memcpy(combinedBuffer, mBuffer, mSize);
    //copy new into buffer after original
    memcpy(combinedBuffer + mSize, other->buffer(), other->size());
    return std::make_shared<ByteStream>(combinedBuffer, combinedSize, true);
}

}
}