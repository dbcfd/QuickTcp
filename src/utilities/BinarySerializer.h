#pragma once
#include "Cache/Platform.h"
#include "Cache/ByteStream.h"

namespace markit {
namespace cache {

/**
 * Class to read and write from a binary representation, which consists of a buffer and size. When writing to binary, buffer will be
 * automatically enlarged to support writes. This will result in a reallocation and copy. Setting appropriate expected sizes will reduce
 * the number of times this occurs. When reading from a buffer, the serializer can take ownership of the buffer to prevent memory leaks.
 */
//------------------------------------------------------------------------------
class CACHE_API BinarySerializer
{
public:
    /**
     * Create a serializer with a buffer of expected size and with no data. This constructor
     * is best used when preparing to write an object to binary.
     * @param expectedSize Expected size of buffer. Initial buffer size will be set to this. Correct buffer sizes reduce allocations
     */
    BinarySerializer(size_t expectedSize = 1000);
    /**
     * Create a serializer from a buffer and size. This constructor is best used when preparing
     * to set an object's state from a buffer. The buffer is copied with this constructor.
     * @param buffer Buffer to copy from
     * @param size Size of buffer
     */
    BinarySerializer(const void* buffer, size_t size);
    /**
     * Create a serializer from a buffer and size. This constructor is best used when preparing
     * to set an object's state from a buffer. The buffer may simply be owned dependent on parameters.
     * @param buffer Buffer to own or copy
     * @param size Size of buffer
     * @param takeOwnershipOfBuffer Whether the buffer should be copied or owned
     */
    BinarySerializer(void* buffer, size_t size, const bool takeOwnershipOfBuffer);
    ~BinarySerializer();

    bool WriteString(const std::string& str);
    bool ReadString(std::string& str);

    template<class T>
    bool WriteT(const T& obj);
    template<class T>
    bool WriteT(const T* obj, const size_t arraySize);
    template<class T>
    bool WriteT(const T& obj, const size_t objSize, const size_t objCount);

    template<class T>
    bool ReadT(T& obj);
    template<class T>
    bool ReadT(T* obj, const size_t arraySize);
    template<class T>
    bool ReadT(T& obj, const size_t objSize, const size_t objCount);

    inline void Skip(const size_t size);
    inline bool ReadComplete() const;
    inline const void* Buffer() const;
    inline size_t Size() const;
    inline size_t BytesRead() const;
    inline void* TransferBuffer();
    inline void ResetPosition();
private:
    char* mPosition;
    char* mBuffer;
    size_t mSize;
    size_t mAllocatedSize;
};

//Inline implementations
//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::WriteT(const T& obj)
{
    return WriteT(obj, sizeof(obj), 1);
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::WriteT(const T* obj, const size_t arraySize)
{
    return (0 != obj && WriteT(*obj, sizeof(T), arraySize));
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::WriteT(const T& obj, const size_t objSize, const size_t objCount)
{
    bool ret = (0 != mBuffer);
    if(ret)
    {
        size_t writeSize = sizeof(char) * objSize * objCount;
        size_t sizeRequired = mSize + writeSize;
        if(mAllocatedSize < sizeRequired)
        {
            size_t newSize = mAllocatedSize * 2;
            if(sizeRequired > newSize)
            {
                newSize = sizeRequired;
            }
            mAllocatedSize = newSize;
            char* resizedBuffer = (char*)malloc(sizeof(char) * mAllocatedSize);
            memset(resizedBuffer, 0, mAllocatedSize);
            memcpy(resizedBuffer, mBuffer, mSize);
            free(mBuffer);
            mBuffer = resizedBuffer;
            mPosition = mBuffer + mSize;
        }
        memcpy(mPosition, &obj, writeSize);
        mPosition += writeSize;
        mSize += writeSize;
    }
    return ret;
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::ReadT(T& obj)
{
    return ReadT(obj, sizeof(obj), 1);
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::ReadT(T* obj, const size_t arraySize)
{
    return (0 != obj && ReadT(*obj, sizeof(T), arraySize));
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::ReadT(T& obj, const size_t objSize, const size_t objCount)
{
    size_t sizeUtilized = mPosition - mBuffer;
    size_t sizeRequired = objSize * objCount;
    bool ret = (mSize - sizeUtilized >= sizeRequired);
    if(ret)
    {
        memcpy(&obj, mPosition, sizeRequired);
        mPosition += sizeRequired;
    }
    return ret;
}

//------------------------------------------------------------------------------
void BinarySerializer::Skip(const size_t count)
{
    size_t sizeRead = mPosition - mBuffer;
    size_t maxSkip = std::min(mSize - sizeRead, count);
    mPosition += maxSkip;
}

//------------------------------------------------------------------------------
bool BinarySerializer::ReadComplete() const
{
    return (mBuffer + mSize == mPosition);
}

//------------------------------------------------------------------------------
const void* BinarySerializer::Buffer() const
{
    return (void*)mBuffer;
}

//------------------------------------------------------------------------------
size_t BinarySerializer::Size() const
{
    return mSize;
}

//------------------------------------------------------------------------------
size_t BinarySerializer::BytesRead() const
{
    return (mPosition - mBuffer);
}

//------------------------------------------------------------------------------
void* BinarySerializer::TransferBuffer()
{
    void* retBuffer = mBuffer;
    mBuffer = nullptr;
    mSize = 0;
    mAllocatedSize = 0;
    mPosition = 0;
    return retBuffer;
}

//------------------------------------------------------------------------------
void BinarySerializer::ResetPosition()
{
    mPosition = mBuffer;
}

}
}
