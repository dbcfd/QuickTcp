#include "utilities/BinarySerializable.h"
#include "utilities/ByteStream.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp::utilities;

class MyObj : public BinarySerializable
{
public:
	MyObj(const int intVal, const double doubleVal, const std::string& stringVal, float* floatArrayVal, size_t floatArraySize)
		: BinarySerializable(),
		mInt(intVal), mDouble(doubleVal), mString(stringVal), mFloatArraySize(floatArraySize)
	{
		mFloatArray = (float*)malloc(sizeof(float) * mFloatArraySize);
		for(size_t i = 0; i < floatArraySize; ++i)
		{
			mFloatArray[i] = floatArrayVal[i];
		}
		
	}
	MyObj() : mFloatArray(nullptr), mFloatArraySize(0)
	{

	}
	MyObj(const ByteStream& str) : BinarySerializable(), mFloatArray(nullptr), mFloatArraySize(0)
	{
		fromByteStream(str);
	}
	virtual ~MyObj()
	{
		free(mFloatArray);
	}

	virtual void readFromStream(BinarySerializer& serializer)
	{
		serializer.readT<int>(mInt);
		serializer.readT<double>(mDouble);
		serializer.readString(mString);
		serializer.readT<size_t>(mFloatArraySize);
		free(mFloatArray);
		mFloatArray = (float*)malloc(sizeof(float)*mFloatArraySize);
		serializer.readT<float>(mFloatArray, mFloatArraySize);
	}

	virtual void writeToStream(BinarySerializer& serializer)
	{
		serializer.writeT<int>(mInt);
		serializer.writeT<double>(mDouble);
		serializer.writeString(mString);
		serializer.writeT<size_t>(mFloatArraySize);
		serializer.writeT<float>(mFloatArray, mFloatArraySize);
	}

	int mInt;
	double mDouble;
	std::string mString;
	float* mFloatArray;
	size_t mFloatArraySize;
};

static int intVal = 2;
static double doubleVal = 6.5789;
static const char* stringVal = "some string\0";
static size_t stringSize = strlen(stringVal);
static float floatVal[] =  {0.56f, 0.18f, 0.29f, 0.44f, 0.90f};
static size_t floatSize = 5;

class BinarySerializableTest : public testing::Test
{
public:
	BinarySerializableTest() : file(nullptr), fileSize(0)
	{

	}

	virtual void SetUp()
	{
		ASSERT_EQ(0, tmpfile_s(&file));

		fwrite(&intVal, sizeof(int), 1, file);
		fwrite(&doubleVal, sizeof(double), 1, file);
		fwrite(&stringSize, sizeof(size_t), 1, file);
		fwrite(&stringVal[0], sizeof(char), stringSize, file);
		fwrite(&floatSize, sizeof(size_t), 1, file);
		fwrite(&floatVal[0], sizeof(float), floatSize, file);

		fileSize = ftell(file);

		rewind(file);

		byteStream.resize(fileSize / sizeof(char), 0);
		fread(&(byteStream[0]), sizeof(char), fileSize, file);

		rewind(file);
	}

	virtual void TearDown()
	{
		if(nullptr != file)
		{
			fclose(file);
			file = nullptr;
		}
		byteStream.clear();
	}

	FILE* file;
	size_t fileSize;
	std::vector<char> byteStream;
};

TEST_F(BinarySerializableTest, READ)
{	
	{
		//test a read directly from file
		MyObj testObj;
		ASSERT_NO_THROW(testObj.fromBinaryStream(file));

		EXPECT_EQ(intVal, testObj.mInt);
		EXPECT_EQ(doubleVal, testObj.mDouble);
		EXPECT_STREQ(stringVal, testObj.mString.c_str());
		ASSERT_EQ(floatSize, testObj.mFloatArraySize);
		for(size_t i = 0; i < floatSize; ++i)
		{
			EXPECT_EQ(floatVal[i], testObj.mFloatArray[i]);
		}
	}

	{
		//test a read from a byte stream
		MyObj* testObj =  nullptr;
		ASSERT_NO_THROW(testObj = new MyObj(byteStream));
		ASSERT_NE(nullptr, testObj);

		EXPECT_EQ(intVal, testObj->mInt);
		EXPECT_EQ(doubleVal, testObj->mDouble);
		EXPECT_STREQ(stringVal, testObj->mString.c_str());
		ASSERT_EQ(floatSize, testObj->mFloatArraySize);
		for(size_t i = 0; i < floatSize; ++i)
		{
			EXPECT_EQ(floatVal[i], testObj->mFloatArray[i]);
		}
        delete testObj;
	}	
}

TEST_F(BinarySerializableTest, WRITE)
{
	MyObj testObj(intVal, doubleVal, stringVal, floatVal, floatSize);

	FILE* testFile = nullptr;
	ASSERT_NO_THROW(testFile = testObj.toBinaryStream());
    fseek(testFile, 0, SEEK_END);
	size_t testFileSize = ftell(testFile);
	ASSERT_EQ(fileSize, testFileSize);
	rewind(testFile);

	int testInt;
	double testDouble;
	std::string testString;
	size_t testStringSize, testFloatSize;
	float testFloat[5];

	fread(&testInt, sizeof(int), 1, testFile);
	fread(&testDouble, sizeof(double), 1, testFile);
	fread(&testStringSize, sizeof(size_t), 1, testFile);
	testString.resize(testStringSize, 0);
	fread(&(testString[0]), sizeof(char), testStringSize, testFile);
	fread(&testFloatSize, sizeof(size_t), 1, testFile);
	fread(testFloat, sizeof(float), testFloatSize, testFile);

	EXPECT_EQ(intVal, testInt);
	EXPECT_EQ(doubleVal, testDouble);
	EXPECT_STREQ(stringVal, testString.c_str());
	ASSERT_EQ(floatSize, testFloatSize);
	for(size_t i = 0; i < floatSize; ++i)
	{
		EXPECT_EQ(floatVal[i], testFloat[i]);
	}

	ByteStream testByteStream;
	ASSERT_NO_THROW(testByteStream = testObj.toByteStream());
	ASSERT_EQ(byteStream.size(), testByteStream.getSize());
	for(size_t i = 0; i < byteStream.size(); ++i)
	{
		EXPECT_EQ(byteStream[i], testByteStream.getData()[i]);
	}
}

TEST_F(BinarySerializableTest, BAD_READ)
{
    {
        FILE* file = nullptr;
        tmpfile_s(&file);

        fwrite(&intVal, sizeof(int), 1, file);
	    fwrite(&doubleVal, sizeof(double), 1, file);
        fwrite(&doubleVal, sizeof(double), 1, file);
	    fwrite(&stringSize, sizeof(size_t), 1, file);
	    fwrite(&stringVal[0], sizeof(char), stringSize, file);
	    fwrite(&floatSize, sizeof(size_t), 1, file);
	    fwrite(&floatVal[0], sizeof(float), floatSize, file);

        MyObj testObj;
        EXPECT_THROW(testObj.fromBinaryStream(file), std::runtime_error);
    }

    {
        FILE* file = nullptr;
        tmpfile_s(&file);

        size_t badStringSize = stringSize - 2;

        fwrite(&intVal, sizeof(int), 1, file);
        fwrite(&doubleVal, sizeof(double), 1, file);
        fwrite(&badStringSize, sizeof(size_t), 1, file);
	    fwrite(&stringVal[0], sizeof(char), stringSize, file);
	    fwrite(&floatSize, sizeof(size_t), 1, file);
	    fwrite(&floatVal[0], sizeof(float), floatSize, file);

        MyObj testObj;
        EXPECT_THROW(testObj.fromBinaryStream(file), std::runtime_error);
    }

    {
        FILE* file = nullptr;
        tmpfile_s(&file);

        fwrite(&intVal, sizeof(int), 1, file);
	    fwrite(&doubleVal, sizeof(double), 1, file);
	    fwrite(&stringSize, sizeof(size_t), 1, file);
	    fwrite(&stringVal[0], sizeof(char), stringSize, file);
	    fwrite(&floatSize, sizeof(size_t), 1, file);
	    fwrite(&floatVal[0], sizeof(float), floatSize, file);
        fwrite(&doubleVal, sizeof(double), 1, file);

        MyObj testObj;
        EXPECT_THROW(testObj.fromBinaryStream(file), std::runtime_error);
    }

}