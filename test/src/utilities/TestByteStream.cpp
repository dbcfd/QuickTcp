#include "utilities/ByteStream.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp::utilities;

TEST(BYTESTREAM_TEST, CONSTRUCTORS)
{	
	char input[4] = {'a', 'c', 'k', 'o'};
	std::vector<char> inputVec(4);
	for(size_t i = 0; i < 4; ++i)
	{
		inputVec[i] = input[i];
	}

	ByteStream fromArray(input, 4);
	ASSERT_EQ(4, fromArray.getSize());
	for(size_t i = 0; i < 4; ++i)
	{
		EXPECT_EQ(input[i], fromArray.getBuffer()[i]);
		EXPECT_EQ(inputVec[i], fromArray.getData()[i]);
	}

	ByteStream fromVec(inputVec);
	ASSERT_EQ(4, fromVec.getSize());
	for(size_t i = 0; i < 4; ++i)
	{
		EXPECT_EQ(input[i], fromVec.getBuffer()[i]);
		EXPECT_EQ(inputVec[i], fromVec.getData()[i]);
	}
}