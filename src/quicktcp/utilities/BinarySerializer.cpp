#include "quicktcp/utilities/BinarySerializer.h"

namespace quicktcp {
namespace utilities {

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(size_t expectedSize) : mAllocatedSize(expectedSize), mSize(0)
{
    if(0 == mAllocatedSize)
    {
        mAllocatedSize = 1000;
    }
    mBuffer = (char*)malloc(sizeof(char) * mAllocatedSize);
    memset(mBuffer, 0, mAllocatedSize);
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(const void* buffer, size_t size) : mSize(size), mAllocatedSize(size)
{
    if(nullptr == buffer)
    {
        throw(std::runtime_error("BinarySerializer: construction from nullptr"));
    }
    mBuffer = (char*)malloc(sizeof(char) * size);
    memset(mBuffer, 0, size);
    memcpy(mBuffer, buffer, size);
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::BinarySerializer(void* buffer, size_t size, const bool takeOwnershipOfBuffer) : mSize(size), mAllocatedSize(size)
{
    if(nullptr == buffer)
    {
        throw(std::runtime_error("BinarySerializer: construction from nullptr"));
    }
    if(takeOwnershipOfBuffer)
    {
        mBuffer = (char*)buffer;
    }
    else
    {
        mBuffer = (char*)malloc(sizeof(char) * size);
        memcpy(mBuffer, buffer, size);
    }
    mPosition = mBuffer;
}

//------------------------------------------------------------------------------
BinarySerializer::~BinarySerializer()
{
    free(mBuffer);
}

//------------------------------------------------------------------------------
bool BinarySerializer::writeString(const std::string& str)
{
    size_t len = str.size();
    bool ret = writeT<size_t>(len);
    if(ret && 0 != len)
    {
        ret = writeT<char>(str[0], sizeof(char), len);
    }
    return ret;
}

//------------------------------------------------------------------------------
bool BinarySerializer::readString(std::string& str)
{
    size_t len = 0;
    bool ret = readT<size_t>(len);
    ret = (std::string::npos != len);
    if(ret && 0 != len)
    {
        try
        {
            str.resize(len, 0);
            ret = readT<char>(str[0], sizeof(char), len);
        }
        catch(std::bad_alloc&)
        {
            ret = false;
        }
    }
    return ret;
}

}
}
