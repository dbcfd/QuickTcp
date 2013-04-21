#include "quicktcp/utilities/BinarySerializer.h"
#include "quicktcp/utilities/ISerializable.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

static int intVal = 2;
static double doubleVal = 6.5789;
static const char* stringVal = "some string\0";
static size_t stringSize = strlen(stringVal);
static float floatVal[] =  {0.56f, 0.18f, 0.29f, 0.44f, 0.90f};
static size_t floatSize = 5;

using namespace quicktcp::utilities;

class BCOTestObject : public ISerializable
{
public:
	BCOTestObject(const int _intVal, const double _doubleVal, const std::string& _stringVal, float* floatArrayVal, size_t floatArraySize)
		: ISerializable(),
		mInt(_intVal), mDouble(_doubleVal), mString(_stringVal), mFloatArraySize(floatArraySize)
	{
		mFloatArray = (float*)malloc(sizeof(float) * mFloatArraySize);
		for(size_t i = 0; i < floatArraySize; ++i)
		{
			mFloatArray[i] = floatArrayVal[i];
		}

	}
	BCOTestObject(const int version) : ISerializable(), mFloatArray(0), mFloatArraySize(0)
	{

	}
    BCOTestObject() : ISerializable(), mInt(intVal), mDouble(doubleVal), mString(stringVal), mFloatArraySize(floatSize)
    {
        mFloatArray = (float*)malloc(sizeof(float) * mFloatArraySize);
		for(size_t i = 0; i < floatSize; ++i)
		{
			mFloatArray[i] = floatVal[i];
		}
    }
	virtual ~BCOTestObject()
	{
        if(0 != mFloatArray)
        {
		    free(mFloatArray);
        }
	}

	virtual bool readBinary(BinarySerializer& serializer) final
	{
		bool ret = serializer.readT<int>(mInt);
		ret = ret && serializer.readT<double>(mDouble);
		ret = ret && serializer.readString(mString);
		ret = ret && serializer.readT<size_t>(mFloatArraySize);
        if(0 != mFloatArray)
        {
		    free(mFloatArray);
            mFloatArray = 0;
        }
        if(0 == mFloatArray)
        {
            try {
		        mFloatArray = (float*)malloc(sizeof(float)*mFloatArraySize);
                ret = ret && serializer.readT<float>(mFloatArray, mFloatArraySize);
            }
            catch(std::bad_alloc&)
            {
                ret = false;
            }
        }
        return ret;
	}

	virtual void writeBinary(BinarySerializer& serializer) const final
	{
		serializer.writeT<int>(mInt);
		serializer.writeT<double>(mDouble);
		serializer.writeString(mString);
		serializer.writeT<size_t>(mFloatArraySize);
        if(0 != mFloatArraySize)
        {
		    serializer.writeT<float>(mFloatArray, mFloatArraySize);
        }
	}

    virtual size_t estimateSize() const final
    {
        return sizeof(int) + sizeof(double) + sizeof(size_t) + mString.size() + sizeof(size_t) + sizeof(float)*mFloatArraySize;
    }

    int mInt;
	double mDouble;
	std::string mString;
	float* mFloatArray;
	size_t mFloatArraySize;
};

class BCOChildTestObject : public ISerializable
{
public:
    BCOChildTestObject(const std::string& stringVal, float* floatArrayVal, size_t floatArraySize)
		: ISerializable(), mString(stringVal), mFloatArraySize(floatArraySize)
	{
		mFloatArray = (float*)malloc(sizeof(float) * mFloatArraySize);
		for(size_t i = 0; i < floatArraySize; ++i)
		{
			mFloatArray[i] = floatArrayVal[i];
		}

	}
	BCOChildTestObject() : ISerializable(), mFloatArray(0), mFloatArraySize(0)
	{

	}
	virtual ~BCOChildTestObject()
	{
        if(0 != mFloatArray)
        {
		    free(mFloatArray);
        }
	}

	virtual bool readBinary(BinarySerializer& serializer) final
	{
		bool ret = serializer.readString(mString);
		ret = ret && serializer.readT<size_t>(mFloatArraySize);
        if(0 != mFloatArray)
        {
		    free(mFloatArray);
            mFloatArray = 0;
        }
        if(0 == mFloatArray)
        {
		    mFloatArray = (float*)malloc(sizeof(float)*mFloatArraySize);
        }
		ret = ret && serializer.readT<float>(mFloatArray, mFloatArraySize);
        return ret;
	}

	virtual void writeBinary(BinarySerializer& serializer) const final
	{
		serializer.writeString(mString);
		serializer.writeT<size_t>(mFloatArraySize);
        if(0 != mFloatArraySize)
        {
		    serializer.writeT<float>(mFloatArray, mFloatArraySize);
        }
	}

    virtual size_t estimateSize() const final
    {
        return mString.size() + sizeof(float) * mFloatArraySize;
    }

	std::string mString;
	float* mFloatArray;
	size_t mFloatArraySize;
};

class BCOParentTestObject : public ISerializable
{
public: 
    BCOParentTestObject(const int intVal, const double doubleVal)
		: ISerializable(),
		mInt(intVal), mDouble(doubleVal)
	{

	}
	BCOParentTestObject() : ISerializable()
	{

	}
	virtual ~BCOParentTestObject()
	{
        for(std::vector<BCOChildTestObject*>::iterator iter = mChildren.begin(); iter != mChildren.end(); ++iter)
        {
            delete (*iter);
        }
	}

	virtual bool readBinary(BinarySerializer& serializer) final
	{
		bool ret = serializer.readT<int>(mInt);
		ret = ret && serializer.readT<double>(mDouble);
		size_t len = 0;
        ret = ret && serializer.readT<size_t>(len);
        if(ret && len > 0)
        {
            for(size_t i = 0; ret && i < len; ++i)
            {
                auto obj = std::make_shared<BCOChildTestObject>();
                ret = ret && obj->readBinary(serializer);
                if(ret) mChildren.emplace_back(obj);
            }
        }
        return ret;
	}

	virtual void writeBinary(BinarySerializer& serializer) const final
	{
		serializer.writeT<int>(mInt);
		serializer.writeT<double>(mDouble);
        serializer.writeT<size_t>(mChildren.size());
        for(auto child : mChildren)
        {
            child->writeBinary(serializer);
        }
	}

    virtual size_t estimateSize() const final
    {
        return sizeof(int) + sizeof(double) + sizeof(size_t) + sizeof(BCOChildTestObject) * mChildren.size();
    }

	int mInt;
	double mDouble;
    std::vector<std::shared_ptr<BCOChildTestObject>> mChildren;
};

class SerializableTest : public testing::Test
{
public:
	SerializableTest()
	{

	}

	virtual void SetUp()
	{
        bufferSize = sizeof(int) + sizeof(double) + 2*sizeof(size_t) + sizeof(char)*stringSize + sizeof(float)*floatSize;

        buffer = (char*)malloc(sizeof(char) * bufferSize);
        char* position = buffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &stringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;

        ASSERT_EQ(bufferSize, position - buffer);
	}

	virtual void TearDown()
	{
        free(buffer);
	}

    char* buffer;
    size_t bufferSize;
};

TEST_F(SerializableTest, READ_FROM)
{	
    BinarySerializer serializer(buffer, bufferSize);
	BCOTestObject testObj;
    ASSERT_TRUE(testObj.readBinary(serializer));

	EXPECT_EQ(intVal, testObj.mInt);
	EXPECT_EQ(doubleVal, testObj.mDouble);
	EXPECT_STREQ(stringVal, testObj.mString.c_str());
	ASSERT_EQ(floatSize, testObj.mFloatArraySize);
	for(size_t i = 0; i < floatSize; ++i)
	{
		EXPECT_EQ(floatVal[i], testObj.mFloatArray[i]);
	}
}

TEST_F(SerializableTest, WRITE_TO)
{
	BCOTestObject testObj(intVal, doubleVal, stringVal, floatVal, floatSize);

    BinarySerializer serializer;
    testObj.writeBinary(serializer);

    ASSERT_EQ(bufferSize, serializer.size());

    for(size_t i = 0; i < bufferSize; ++i)
    {
        EXPECT_EQ(buffer[i], ( (char*) serializer.buffer() )[i]);
    }
}

TEST_F(SerializableTest, BAD_READ_FROM)
{
    {
        size_t badBufferSize = sizeof(int) + 2*sizeof(double) + 2*sizeof(size_t) + sizeof(char)*stringSize + sizeof(float)*floatSize;

        char* badBuffer = (char*)malloc(sizeof(char) * badBufferSize);
        char* position = badBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &stringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;

        BinarySerializer serializer(badBuffer, badBufferSize, true);
        BCOTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }

    {
        size_t goodBufferSize = sizeof(int) + sizeof(double) + 2*sizeof(size_t) + sizeof(char)*stringSize + sizeof(float)*floatSize;

        char* goodBuffer = (char*)malloc(sizeof(char) * goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        size_t badStringSize = stringSize - 2;
        memcpy(position, &badStringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;

        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }

    {
        size_t goodBufferSize = sizeof(int) + 2*sizeof(double) + 2*sizeof(size_t) + sizeof(char)*stringSize + sizeof(float)*floatSize;

        char* goodBuffer = (char*)malloc(sizeof(char) * goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &stringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);

        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }

    {
        size_t goodBufferSize = sizeof(int) + sizeof(double) + 2*sizeof(size_t) + sizeof(char)*stringSize + sizeof(float)*floatSize;

        char* goodBuffer = (char*)malloc(sizeof(char) * goodBufferSize);
        memset(goodBuffer, 0, goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        std::string badString("bad string");
        size_t badStringSize = badString.size();
        memcpy(position, &badStringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &badString[0], sizeof(char)*badStringSize);
        position += sizeof(char)*badStringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;

        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }
}

TEST_F(SerializableTest, DATA_CHILD_OBJECTS)
{
	BCOParentTestObject parentObj(intVal, doubleVal);
    parentObj.mChildren.emplace_back(std::make_shared<BCOChildTestObject>(stringVal, floatVal, floatSize));
    parentObj.mChildren.emplace_back(std::make_shared<BCOChildTestObject>(stringVal, floatVal, floatSize));

    BinarySerializer serializer;
    parentObj.writeBinary(serializer);

    size_t goodBufferSize = sizeof(int) + sizeof(double) + sizeof(size_t) + 
        2*sizeof(size_t) + 2*sizeof(char)*stringSize + 2*sizeof(float)*floatSize;

    char* goodBuffer = (char*)malloc(goodBufferSize);
    char* position = goodBuffer;
    size_t numChildren = 2;

    memcpy(position, &intVal, sizeof(int));
    position += sizeof(int);
    memcpy(position, &doubleVal, sizeof(double));
    position += sizeof(double);
    memcpy(position, &numChildren, sizeof(size_t));
    position += sizeof(size_t);
    for(size_t i = 0; i < numChildren; ++i)
    {
        memcpy(position, &stringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;
    }

    ASSERT_EQ(position - goodBuffer, serializer.size());
    for(size_t i = 0; i < goodBufferSize; ++i)
    {
        char g = goodBuffer[i];
        char t = ((char*)serializer.buffer())[i];
        EXPECT_EQ(goodBuffer[i], ((char*)serializer.buffer())[i]);
    }
}

TEST_F(SerializableTest, READ_FROM_CHILD_OBJECTS)
{
    static const size_t NUM_CHILDREN = 2;
    size_t goodBufferSize = sizeof(int) + sizeof(double) + sizeof(size_t) + NUM_CHILDREN*sizeof(size_t) +
        NUM_CHILDREN*sizeof(char)*stringSize + NUM_CHILDREN*sizeof(size_t) + NUM_CHILDREN*sizeof(float)*floatSize;

    char* goodBuffer = (char*)malloc(goodBufferSize);
    char* position = goodBuffer;    

    memcpy(position, &intVal, sizeof(int));
    position += sizeof(int);
    memcpy(position, &doubleVal, sizeof(double));
    position += sizeof(double);
    memcpy(position, &NUM_CHILDREN, sizeof(size_t));
    position += sizeof(size_t);
    for(size_t i = 0; i < NUM_CHILDREN; ++i)
    {
        memcpy(position, &stringSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &stringVal[0], sizeof(char)*stringSize);
        position += sizeof(char)*stringSize;
        memcpy(position, &floatSize, sizeof(size_t));
        position += sizeof(size_t);
        memcpy(position, &floatVal[0], sizeof(float)*floatSize);
        position += sizeof(float)*floatSize;
    }

    BinarySerializer serializer(goodBuffer, goodBufferSize, true);
    BCOParentTestObject parentObj;
    ASSERT_TRUE(parentObj.readBinary(serializer));
    EXPECT_EQ(intVal, parentObj.mInt);
    EXPECT_EQ(doubleVal, parentObj.mDouble);
    EXPECT_EQ(NUM_CHILDREN, parentObj.mChildren.size());
    for(size_t i = 0; i < NUM_CHILDREN; ++i)
    {
        BCOChildTestObject* child = parentObj.mChildren[i];
        EXPECT_EQ(stringVal, child->mString);
        ASSERT_EQ(floatSize, child->mFloatArraySize);
        for(size_t j = 0; j < floatSize; ++j)
        {
            EXPECT_EQ(floatVal[j], child->mFloatArray[j]);
        }
    }
}

TEST_F(SerializableTest, BAD_READ_FROM_CHILD)
{
    static const size_t NUM_CHILDREN = 2;
    {
        size_t goodBufferSize = sizeof(int) + 2*sizeof(double) + sizeof(size_t) + NUM_CHILDREN*sizeof(size_t) +
            NUM_CHILDREN*sizeof(char)*stringSize + NUM_CHILDREN*sizeof(size_t) + NUM_CHILDREN*sizeof(float)*floatSize;

        char* goodBuffer = (char*)malloc(goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &doubleVal, sizeof(double)); //bad write here
        position += sizeof(double);
        memcpy(position, &NUM_CHILDREN, sizeof(size_t));
        position += sizeof(size_t);
        for(size_t i = 0; i < NUM_CHILDREN; ++i)
        {
            memcpy(position, &stringSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &stringVal[0], sizeof(char)*stringSize);
            position += sizeof(char)*stringSize;
            memcpy(position, &floatSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &floatVal[0], sizeof(float)*floatSize);
            position += sizeof(float)*floatSize;
        }

        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOParentTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }

    {
        size_t goodBufferSize = sizeof(int) + sizeof(double) + sizeof(size_t) + NUM_CHILDREN*sizeof(size_t) +
            NUM_CHILDREN*sizeof(char)*stringSize + NUM_CHILDREN*sizeof(size_t) + NUM_CHILDREN*sizeof(float)*floatSize +
            NUM_CHILDREN*sizeof(double);

        char* goodBuffer = (char*)malloc(goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &NUM_CHILDREN, sizeof(size_t));
        position += sizeof(size_t);
        for(size_t i = 0; i < NUM_CHILDREN; ++i)
        {
            memcpy(position, &stringSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &stringVal[0], sizeof(char)*stringSize);
            position += sizeof(char)*stringSize;
            memcpy(position, &floatSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &floatVal[0], sizeof(float)*floatSize);
            position += sizeof(float)*floatSize;
            memcpy(position, &doubleVal, sizeof(double)); //bad write here
            position += sizeof(double);
        }

        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOParentTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }

    {
        size_t goodBufferSize = sizeof(int) + sizeof(double) + sizeof(size_t) + NUM_CHILDREN*sizeof(size_t) +
            NUM_CHILDREN*sizeof(char)*stringSize + NUM_CHILDREN*sizeof(size_t) + NUM_CHILDREN*sizeof(float)*floatSize;

        char* goodBuffer = (char*)malloc(goodBufferSize);
        char* position = goodBuffer;

        memcpy(position, &intVal, sizeof(int));
        position += sizeof(int);
        memcpy(position, &doubleVal, sizeof(double));
        position += sizeof(double);
        memcpy(position, &NUM_CHILDREN, sizeof(size_t));
        position += sizeof(size_t);
        size_t badStringSize = stringSize - 2;
        for(size_t i = 0; i < NUM_CHILDREN; ++i)
        {
            memcpy(position, &badStringSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &stringVal[0], sizeof(char)*stringSize);
            position += sizeof(char)*stringSize;
            memcpy(position, &floatSize, sizeof(size_t));
            position += sizeof(size_t);
            memcpy(position, &floatVal[0], sizeof(float)*floatSize);
            position += sizeof(float)*floatSize;
        }
        
        BinarySerializer serializer(goodBuffer, goodBufferSize, true);
        BCOParentTestObject testObj;
        EXPECT_FALSE(testObj.readBinary(serializer));
    }
}

class SuperSizedBCO : public ISerializable
{
public:
    SuperSizedBCO() : ISerializable(),
        mString("this is some long string that should exceed double the expected size, since we start the expected size of the object at only 5. We should have code that handles this case")
    {

    }

    virtual bool readBinary(BinarySerializer& serializer) final
	{
		return serializer.readString(mString);
	}

	virtual void writeBinary(BinarySerializer& serializer) const final
	{
        serializer.writeString(mString);
	}

    virtual size_t estimateSize() const final
    {
        return size_t(5);
    }

    std::string mString;
};

TEST_F(SerializableTest, WRITTEN_OBJ_EXCEEDS_DOUBLE_EXPECTED)
{
    SuperSizedBCO obj;

    BinarySerializer serializer;
    obj.writeBinary(serializer);

    size_t stringSize = obj.mString.size();
    size_t expectedBufferSize = sizeof(size_t) + sizeof(char)*stringSize;

    char* expectedBuffer = (char*)malloc(expectedBufferSize);
    char* position = expectedBuffer;

    int version = 1;
    memcpy(position, &stringSize, sizeof(size_t));
    position += sizeof(size_t);
    memcpy(position, &obj.mString[0], sizeof(char)*stringSize);
    position += sizeof(char)*stringSize;

    ASSERT_EQ(expectedBufferSize, serializer.size());

    for(size_t i = 0; i < expectedBufferSize; ++i)
    {
        EXPECT_EQ(expectedBuffer[i], ( (char*)serializer.buffer() )[i]);
    }

    free(expectedBuffer);
}
