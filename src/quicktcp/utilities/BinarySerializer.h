#pragma once
#include "quicktcp/utilities/Platform.h"
#include "quicktcp/utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

/**
 * Class to read and write from a binary representation, which consists of a buffer and size. When writing to binary, buffer will be
 * automatically enlarged to support writes. This will result in a reallocation and copy. Setting appropriate expected sizes will reduce
 * the number of times this occurs. When reading from a buffer, the serializer can take ownership of the buffer to prevent memory leaks.
 */
//------------------------------------------------------------------------------
class UTILITIES_API BinarySerializer
{
public:
    class UTILITIES_API IStringSizeCheck {
    public:
        virtual ~IStringSizeCheck();

        virtual bool isValidStringSize(stream_size_t size) const = 0;
    };

    /**
     * Create a serializer with a buffer of expected size and with no data. This constructor
     * is best used when preparing to write an object to binary.
     * @param expectedSize Expected size of buffer. Initial buffer size will be set to this. Correct buffer sizes reduce allocations
     */
    BinarySerializer(stream_size_t expectedSize = 1000);
    /**
     * Create a serializer from a buffer and size. This constructor is best used when preparing
     * to set an object's state from a buffer. The buffer is copied with this constructor.
     * @param buffer Buffer to copy from
     * @param size Size of buffer
     */
    BinarySerializer(const stream_data_t* buffer, stream_size_t size);
    /**
     * Create a serializer from a buffer and size. This constructor is best used when preparing
     * to set an object's state from a buffer. The buffer may simply be owned dependent on parameters.
     * @param buffer Buffer to own or copy
     * @param size Size of buffer
     * @param takeOwnershipOfBuffer Whether the buffer should be copied or owned
     */
    BinarySerializer(stream_data_t* buffer, stream_size_t size, const bool takeOwnershipOfBuffer);
    ~BinarySerializer();

    bool writeString(const std::string& str);
    bool writeEof();
    bool readString(std::string& str);
    bool readEof();

    template<class T>
    bool writeT(const T& obj);
    template<class T>
    bool writeT(const T* obj, const stream_size_t arraySize);
    template<class T>
    bool writeT(const T& obj, const stream_size_t objSize, const stream_size_t objCount);

    template<class T>
    bool readT(T& obj);
    template<class T>
    bool readT(T* obj, const stream_size_t arraySize);
    template<class T>
    bool readT(T& obj, const stream_size_t objSize, const stream_size_t objCount);

    /**
     * Copy stream from current position to end, into a byte stream
     */
    std::shared_ptr<utilities::ByteStream> toStream() const;
    /**
     * Transfer entire stream into a byte stream
     */
    std::shared_ptr<utilities::ByteStream> transferToStream();

    inline void skip(const stream_size_t size);
    inline bool readComplete() const;
    inline const stream_data_t* buffer() const;
    inline stream_size_t size() const;
    inline stream_size_t bytesRead() const;
    inline stream_data_t* transferBuffer();
    inline void resetPosition();
    inline void checkStringSize(std::shared_ptr<IStringSizeCheck> checker);
private:
    stream_data_t* mPosition;
    stream_data_t* mBuffer;
    stream_size_t mSize;
    stream_size_t mAllocatedSize;
    std::shared_ptr<IStringSizeCheck> mStringSizeCheck;
};

//Inline implementations
//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::writeT(const T& obj)
{
    return writeT(obj, (stream_size_t)sizeof(obj), 1);
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::writeT(const T* obj, const stream_size_t arraySize)
{
    return (0 != obj && writeT(*obj, (stream_size_t)sizeof(T), arraySize));
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::writeT(const T& obj, const stream_size_t objSize, const stream_size_t objCount)
{
    bool ret = (0 != mBuffer);
    if(ret)
    {
        auto writeSize = (stream_size_t)sizeof(char) * objSize * objCount;
        auto sizeRequired = mSize + writeSize;
        if(mAllocatedSize < sizeRequired)
        {
            auto newSize = mAllocatedSize * 2;
            if(sizeRequired > newSize)
            {
                newSize = sizeRequired;
            }
            mAllocatedSize = newSize;
            auto resizedBuffer = new stream_data_t[mAllocatedSize];
            memset(resizedBuffer, 0, mAllocatedSize);
            memcpy(resizedBuffer, mBuffer, mSize);
            delete[] mBuffer;
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
bool BinarySerializer::readT(T& obj)
{
    return readT(obj, sizeof(obj), 1);
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::readT(T* obj, const stream_size_t arraySize)
{
    return (0 != obj && readT(*obj, (stream_size_t)sizeof(T), arraySize));
}

//------------------------------------------------------------------------------
template<class T>
bool BinarySerializer::readT(T& obj, const stream_size_t objSize, const stream_size_t objCount)
{
    auto sizeRequired = objSize * objCount;
    bool ret = (mSize - bytesRead() >= sizeRequired);
    if(ret)
    {
        memcpy(&obj, mPosition, sizeRequired);
        mPosition += sizeRequired;
    }
    return ret;
}

//------------------------------------------------------------------------------
void BinarySerializer::skip(const stream_size_t count)
{
    auto maxSkip = std::min(mSize - bytesRead(), count);
    mPosition += maxSkip;
}

//------------------------------------------------------------------------------
bool BinarySerializer::readComplete() const
{
    return (mBuffer + mSize == mPosition);
}

//------------------------------------------------------------------------------
const stream_data_t* BinarySerializer::buffer() const
{
    return mBuffer;
}

//------------------------------------------------------------------------------
stream_size_t BinarySerializer::size() const
{
    return mSize;
}

//------------------------------------------------------------------------------
stream_size_t BinarySerializer::bytesRead() const
{
    return (stream_size_t)std::distance(mBuffer, mPosition);
}

//------------------------------------------------------------------------------
stream_data_t* BinarySerializer::transferBuffer()
{
    auto retBuffer = mBuffer;
    mBuffer = nullptr;
    mSize = 0;
    mAllocatedSize = 0;
    mPosition = 0;
    return retBuffer;
}

//------------------------------------------------------------------------------
void BinarySerializer::resetPosition()
{
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
void BinarySerializer::checkStringSize(std::shared_ptr<BinarySerializer::IStringSizeCheck> checker)
{
    mStringSizeCheck = checker;
}

}
}
