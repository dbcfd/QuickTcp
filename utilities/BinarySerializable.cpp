#include "utilities/BinarySerializable.h"
#include "utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

BinarySerializable::BinarySerializable() : mFile(nullptr)
{

}

BinarySerializable::~BinarySerializable()
{
	if(nullptr != mFile)
	{
		fclose(mFile);
	}
}

void BinarySerializable::fromByteStream(const ByteStream& byteStream)
{
	if(nullptr != mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
	if(0 == tmpfile_s(&mFile))
	{
        mSerializer.setFile(mFile);
		mSerializer.writeT<char>(byteStream.getBuffer(), byteStream.getSize());
		rewind(mFile);
        readFromStream(mSerializer);
        if(0 != feof(mFile))
        {
            throw(std::runtime_error("BinarySerializer: Attempted to read past end of file"));
        }
        size_t filePos = ftell(mFile);
        fseek(mFile, 0, SEEK_END);
        if(filePos != ftell(mFile))
        {
            throw(std::runtime_error("BinarySerializer: Failed to read all of stream"));
        }
	}
	else
	{
		throw(std::runtime_error("BinarySerializable: Failed to create tmpfile"));
	}
}

void BinarySerializable::fromBinaryStream(FILE* binaryStream)
{
	if(nullptr != mFile)
	{
		fclose(mFile);
	}
	mFile = binaryStream;
	if(nullptr != mFile) 
	{
		rewind(mFile);
        mSerializer.setFile(mFile);
        readFromStream(mSerializer);
        if(0 != feof(mFile))
        {
            throw(std::runtime_error("BinarySerializable: Attempted to read past end of file"));
        }
        size_t filePos = ftell(mFile);
        fseek(mFile, 0, SEEK_END);
        if(filePos != ftell(mFile))
        {
            throw(std::runtime_error("BinarySerializable: Failed to read to end of file"));
        }
	}
}

void BinarySerializable::fillStream()
{
	if(nullptr == mFile)
	{
		if(0 == tmpfile_s(&mFile))
		{
            mSerializer.setFile(mFile);
			writeToStream(mSerializer);
		}
	}
}

ByteStream BinarySerializable::toByteStream()
{
	fillStream();
	std::vector<char> vec;
	if(nullptr != mFile)
	{
		fseek(mFile, 0, SEEK_END);
        size_t size = ftell(mFile);
		vec.resize(size, 0);
        rewind(mFile);
        mSerializer.setFile(mFile);
		mSerializer.readT<char>(&(vec[0]), size);
	}
	return ByteStream(vec);
}

FILE* BinarySerializable::toBinaryStream()
{
	fillStream();
    rewind(mFile);
	return mFile;
}

}
}