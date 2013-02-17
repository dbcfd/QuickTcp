#pragma once

#include "Cache/Platform.h"

namespace markit {
namespace cache {
    
/** 
 * Representation of a set of bytes that come from a blob, network connection, etc. Consists of a buffer with data and the size of that buffer.
 */
class CACHE_API ByteStream {
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
     * Transfer ownership of this bytestream's buffer. Buffer is returned, but member buffer is set to nullptr.
     * @return Buffer that was held by this byte stream. Buffer is allocated with malloc, so should be free'd.
     */
    inline const void* TransferBuffer();
    inline const void* Buffer() const;
    inline const size_t Size() const;
private:
    void* mBuffer;
    size_t mSize;
};

//Inline Implementations
//------------------------------------------------------------------------------
const void* ByteStream::TransferBuffer()
{
    const void* retBuffer = mBuffer;
    mBuffer = nullptr;
    mSize = 0;
    return retBuffer;
}

//------------------------------------------------------------------------------
const void* ByteStream::Buffer() const
{
    return mBuffer;
}

//------------------------------------------------------------------------------
const size_t ByteStream::Size() const
{
    return mSize;
}

}
}