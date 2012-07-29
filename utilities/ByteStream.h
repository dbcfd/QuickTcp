#pragma once

#include "utilities/Platform.h"

#include <vector>

namespace quicktcp {
namespace utilities {

class UTILITIES_API ByteStream {
public:
	ByteStream();
	ByteStream(const ByteStream& other);
	ByteStream(const std::vector<char>& data);
	ByteStream(const char* data, const size_t size);
	~ByteStream();

	ByteStream& operator=(const ByteStream& other);

	inline const std::vector<char>& getData() const;
	inline const char* getBuffer() const;
	inline const size_t getSize() const;
private:
	std::vector<char> mData;
};

//Inline methods
const std::vector<char>& ByteStream::getData() const
{
	return mData;
}

const char* ByteStream::getBuffer() const
{
	return &(mData[0]);
}

const size_t ByteStream::getSize() const
{
	return mData.size();
}

}
}