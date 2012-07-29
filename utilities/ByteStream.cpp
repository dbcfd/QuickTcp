#include "utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

ByteStream::ByteStream()
{

}

ByteStream::ByteStream(const ByteStream& other) : mData(other.mData)
{

}

ByteStream& ByteStream::operator=(const ByteStream& other)
{
	mData = other.mData;
	return *this;
}

ByteStream::ByteStream(const std::vector<char>& data) : mData(data)
{

}

ByteStream::ByteStream(const char* buffer, const size_t size)
{
	mData.reserve(size);
	for(size_t i = 0; i < size; ++i) mData.push_back(buffer[i]);
}

ByteStream::~ByteStream()
{

}

}
}