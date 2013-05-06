#include "quicktcp/utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

//------------------------------------------------------------------------------
ByteStream::ByteStream(const stream_data_t* buffer, const stream_size_t size) : mSize(size)
{
    mBuffer = new stream_data_t[size];
    memcpy(mBuffer, buffer, size);
}

//------------------------------------------------------------------------------
ByteStream::ByteStream(stream_data_t* buffer, const stream_size_t size, const bool takeOwnershipOfBuffer)
{
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
    auto combinedSize = mSize + other->size();
    auto combinedBuffer = new stream_data_t[combinedSize];
    auto bufPos = memcpy(combinedBuffer, mBuffer, mSize);
    memcpy(bufPos, other->buffer(), other->size());
    return std::make_shared<ByteStream>(combinedBuffer, combinedSize, true);
}

}
}