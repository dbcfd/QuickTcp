#include "utilities/BinarySerializable.h"
#include "utilities/ByteStream.h"

#include <gtest/gtest.h>

using namespace quicktcp::utilities;

class MyObj : public BinarySerializable
{
public:
	MyObj(const int intVal, const double doubleVal, const std::string& stringVal, float* floatArrayVal, size_t floatArraySize)
		: BinarySerializable(),
		mInt(intVal), mDouble(doubleVal), mString(stringVal), mFloatArray(floatArrayVal), mFloatArraySize(floatArraySize)
	{
		
	}
	MyObj(const ByteStream& str) : BinarySerializable()
	{
		fromByteStream(str);
	}

	int mInt;
	double mDouble;
	std::string mString;
	float* mFloatArray;
	size_t mFloatArraySize;
};

TEST(BINARYSERIALIZABLE_TEST, READ)
{	
	
}