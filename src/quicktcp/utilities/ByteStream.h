#pragma once

#include "Utilities/Platform.h"

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
    ByteStream(const void* buffer, const size_t size);
    /**
     * Construct a byte stream by copying or taking ownership of a buffer with some size.
     * @param buffer Buffer to copy or own. If owning, buffer should be malloc'd
     * @param size Size of buffer
     * @param takeOwnershipOfBuffer Whether buffer should be copied or own
     */
    ByteStream(void* buffer, const size_t size, const bool takeOwnershipOfBuffer = false);
    ~ByteStream();

    /**
     * Append the contents of another bytestream to this stream, taking ownership of the stream.
     */
    void append(std::shared_ptr<ByteStream> other);

    /**
     * Transfer ownership of this bytestream's buffer. Buffer is returned, but member buffer is set to nullptr.
     * @return Buffer that was held by this byte stream. Buffer is allocated with malloc, so should be free'd.
     */
    inline const void* transferBuffer();
    inline const void* buffer() const;
    inline const size_t size() const;
private:
    void* mBuffer;
    size_t mSize;
};

//Inline Implementations
//------------------------------------------------------------------------------
const void* ByteStream::transferBuffer()
{
    const void* retBuffer = mBuffer;
    mBuffer = nullptr;
    mSize = 0;
    return retBuffer;
}

//------------------------------------------------------------------------------
const void* ByteStream::buffer() const
{
    return mBuffer;
}

//------------------------------------------------------------------------------
const size_t ByteStream::size() const
{
    return mSize;
}

}
}