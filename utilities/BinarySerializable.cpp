#include "utilities/BinarySerializable.h"
#include "utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

BinarySerializable::BinarySerializable() : mFile(nullptr), mSize(0)
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
		mSize = 0;
	}
	if(0 == tmpfile_s(&mFile))
	{
        mSerializer.setFile(mFile, 0);
		mSerializer.writeT<char>(byteStream.getBuffer(), byteStream.getSize());
		mSize = ftell(mFile);
		rewind(mFile);
        mSerializer.setFile(mFile, mSize);
        readFromStream(mSerializer);
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
		mSize = 0;
	}
	mFile = binaryStream;
	if(nullptr != mFile) 
	{
		fseek(mFile, 0, SEEK_END);
		mSize = ftell(mFile);
		rewind(mFile);
        mSerializer.setFile(mFile, mSize);
        readFromStream(mSerializer);
	}
}

void BinarySerializable::fillStream()
{
	if(nullptr == mFile)
	{
		mSize = 0;
		if(0 == tmpfile_s(&mFile))
		{
            mSerializer.setFile(mFile, 0);
			try {
                writeToStream(mSerializer);
			}
			catch(std::runtime_error)
			{
				//failed to write
			}
			mSize = ftell(mFile);
		}
	}
}

ByteStream BinarySerializable::toByteStream()
{
	fillStream();
	std::vector<char> vec;
	if(nullptr != mFile)
	{
		rewind(mFile);
		vec.resize(mSize, 0);
        mSerializer.setFile(mFile, mSize);
		try
		{
			mSerializer.readT<char>(&(vec[0]), mSize);
		}
		catch(std::runtime_error)
		{
			//empty byte stream
		}
	}
	return ByteStream(vec);
}

FILE* BinarySerializable::toBinaryStream()
{
	fillStream();
	return mFile;
}

}
}