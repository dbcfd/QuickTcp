#pragma once

#include "utilities/Platform.h"

#include <stdio.h>

namespace quicktcp {
namespace utilities {

class ByteStream;

class UTILITIES_API BinarySerializer {
public:
    BinarySerializer(FILE* file) : mFile(file)
    {

    }
    BinarySerializer() : mFile(nullptr)
    {

    }

    void setFile(FILE* file)
    {
        mFile = file;
    }

    void writeString(const std::string& str)
	{
		size_t len = str.size();
		writeT<size_t>(len);
		if(0 != len)
		{
			writeT<char>(str[0], sizeof(char), len);
		}
	}

	void readString(std::string& str)
	{
		size_t len = 0;
		readT<size_t>(len);
		if(0 != len)
		{
			str.resize(len, 0);
			readT<char>(str[0], sizeof(char), len);
		}
	}

	template<class T>
	void writeT(const T& obj)
	{
		writeT(obj, sizeof(T), 1);
	}
	template<class T>
	void writeT(const T* obj, size_t count)
	{
		if(nullptr == obj)
		{
			throw(std::runtime_error("BinarySerializable: nullptr when attempting write"));
		}
		writeT(*obj, sizeof(T), count);
	}
	template<class T>
	void writeT(const T& obj, size_t size, size_t count)
	{
		if(nullptr != mFile && count != fwrite(&obj, size, count, mFile))
		{
			throw(std::runtime_error("BinarySerializable: Write failed to write all data"));
		}
	}

	template<class T>
	void readT(T& obj)
	{
		readT(obj, sizeof(T), 1);
	}
	template<class T>
	void readT(T* objArray, size_t count)
	{
		if(nullptr == objArray)
		{
			throw(std::runtime_error("BinarySerializable: Cannot read into nullptr array"));
		}
		readT(*objArray, sizeof(T), count);
	}
	template<class T>
	void readT(T& obj, size_t size, size_t count)
	{
        if(nullptr != mFile)
        {
            if(feof(mFile) != 0)
            {
		        throw(std::runtime_error("BinarySerializable: End of file reached"));
		    }
		    if(count != fread(&obj, size, count, mFile))
		    {
			    throw(std::runtime_error("BinarySerializable: Read did not read enough objects"));
		    }
        }
	}
private:
    FILE* mFile;
};

class UTILITIES_API BinarySerializable {
public :
	BinarySerializable();
	virtual ~BinarySerializable();

	void fromByteStream(const ByteStream& byteStream);
	void fromBinaryStream(FILE* binaryStream);

	ByteStream toByteStream();
	FILE* toBinaryStream();

protected:
	void fillStream();
    virtual void readFromStream(BinarySerializer& serializer) = 0;
    virtual void writeToStream(BinarySerializer& serializer) = 0;

private:
    BinarySerializer mSerializer;
	FILE* mFile;
};

}
}