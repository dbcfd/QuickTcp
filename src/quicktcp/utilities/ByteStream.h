#pragma once

#include "quicktcp/utilities/Platform.h"

#include <iostream>
#include <memory>

namespace quicktcp {
namespace utilities {
    
/** 
 * Representation of a set of bytes that come from a blob, network connection, etc. Consists of a buffer with data and the size of that buffer.
 */
class UTILITIES_API ByteStream {
public:
    /**
     * Construct a byte stream by copying an existing buffer with some size.
     * @param buffer Buffer to copy
     * @param size Size of buffer
     */
    ByteStream(const stream_data_t* buffer, const stream_size_t size);
    /**
     * Construct a byte stream by copying or taking ownership of a buffer with some size.
     * @param buffer Buffer to copy or own. If owning, buffer should be malloc'd
     * @param size Size of buffer
     * @param takeOwnershipOfBuffer Whether buffer should be copied or own
     */
    ByteStream(stream_data_t* buffer, const stream_size_t size, const bool takeOwnershipOfBuffer = false);
    ~ByteStream();

    /**
     * Append the contents of another bytestream to this stream, creating a new stream
     */
    std::shared_ptr<ByteStream> append(std::shared_ptr<ByteStream> other) const;

    /**
     * Transfer ownership of this bytestream's buffer. Buffer is returned, but member buffer is set to nullptr.
     * @return Buffer that was held by this byte stream. Buffer is allocated with malloc, so should be free'd.
     */
    inline stream_data_t* transferBuffer();
    inline const stream_data_t* buffer() const;
    inline const stream_size_t size() const;
    inline const bool hasEof() const;
    inline void appendEof();
    inline void stripEof();
private:
    stream_data_t* mBuffer;
    stream_size_t mSize;
};

//Inline Implementations
//------------------------------------------------------------------------------
stream_data_t* ByteStream::transferBuffer()
{
    auto retBuffer = mBuffer;
    mBuffer = nullptr;
    mSize = 0;
    return retBuffer;
}

//------------------------------------------------------------------------------
const stream_data_t* ByteStream::buffer() const
{
    return mBuffer;
}

//------------------------------------------------------------------------------
const stream_size_t ByteStream::size() const
{
    return mSize;
}

//------------------------------------------------------------------------------
const bool ByteStream::hasEof() const
{
    return (std::ios::eofbit == (int)mBuffer[mSize-1]);
}

//------------------------------------------------------------------------------
void ByteStream::appendEof()
{
    auto eofBuffer = new stream_data_t[mSize+1];
    memcpy(eofBuffer, mBuffer, mSize);
    delete[] mBuffer;
    eofBuffer[mSize] = (stream_data_t)std::ios::eofbit;
    mBuffer = eofBuffer;
    mSize += 1;
}

//------------------------------------------------------------------------------
void ByteStream::stripEof()
{
    auto eofBuffer = new stream_data_t[mSize-1];
    memcpy(eofBuffer, mBuffer, mSize-1);
    delete[] mBuffer;
    mBuffer = eofBuffer;
    mSize -= 1;
}

}
}