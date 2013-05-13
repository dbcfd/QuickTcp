#include "quicktcp/utilities/BinarySerializer.h"

#include <iostream>

namespace quicktcp {
namespace utilities {

//------------------------------------------------------------------------------
BinarySerializer::IStringSizeCheck::~IStringSizeCheck()
{

}

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(stream_size_t expectedSize) : mAllocatedSize(expectedSize), mSize(0)
{
    if(0 == mAllocatedSize)
    {
        mAllocatedSize = 1000;
    }
    mBuffer = new stream_data_t[mAllocatedSize];
    memset(mBuffer, 0, mAllocatedSize);
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(const stream_data_t* buffer, stream_size_t size) : mSize(size), mAllocatedSize(size)
{
    if(nullptr == buffer)
    {
        throw(std::runtime_error("BinarySerializer: construction from nullptr"));
    }
    mBuffer = new stream_data_t[size];
    memcpy(mBuffer, buffer, size);
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(stream_data_t* buffer, stream_size_t size, const bool takeOwnershipOfBuffer) : mSize(size), mAllocatedSize(size)
{
    if(nullptr == buffer)
    {
        throw(std::runtime_error("BinarySerializer: construction from nullptr"));
    }
    if(takeOwnershipOfBuffer)
    {
        mBuffer = buffer;
    }
    else
    {
        mBuffer = new stream_data_t[size];
        memcpy(mBuffer, buffer, size);
    }
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::~BinarySerializer()
{
    delete[] mBuffer;
}

//------------------------------------------------------------------------------
bool BinarySerializer::writeString(const std::string& str)
{
    stream_size_t len = (stream_size_t)str.size();
    bool ret = writeT<stream_size_t>(len);
    if(ret && 0 != len)
    {
        ret = writeT<char>(str[0], (stream_size_t)sizeof(char), len);
    }
    return ret;
}

//------------------------------------------------------------------------------
bool BinarySerializer::readString(std::string& str)
{
    stream_size_t len = 0;
    bool ret = readT<stream_size_t>(len);
    ret = (mStringSizeCheck ? mStringSizeCheck->isValidStringSize(len) : true);
    if(ret)
    {
        str.resize(len, 0);
        ret = readT<char>(str[0], (stream_size_t)sizeof(char), len);
    }
    return ret;
}

//------------------------------------------------------------------------------
bool BinarySerializer::writeEof()
{
    return writeT<int>(std::ios::eofbit);
}

//------------------------------------------------------------------------------
bool BinarySerializer::readEof()
{
    int eof;
    bool ret = readT<int>(eof);
    ret = ret && (std::ios::eofbit == eof);
    return ret;
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> BinarySerializer::toStream() const
{
    return std::make_shared<utilities::ByteStream>(mPosition, mSize - bytesRead());
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> BinarySerializer::transferToStream()
{
    auto ret = std::make_shared<utilities::ByteStream>(mBuffer, mSize, true);
    mBuffer = nullptr;
    mSize = 0;
    return ret;
}

}
}
