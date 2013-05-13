#include "quicktcp/utilities/ByteStream.h"
#include "quicktcp/utilities/BinarySerializer.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp;
using namespace quicktcp::utilities;

TEST(BYTESTREAM, CONSTRUCTOR)
{
    {
        auto buffer = new char[20];
        memset(buffer, 0, 20);
        EXPECT_NO_THROW(ByteStream(buffer, 20));
        delete[] buffer;
    }

    {
        auto buffer = new char[20];
        memset(buffer, 0, 20);
        EXPECT_NO_THROW(ByteStream(buffer, 20, true));
    }
}

#ifdef _DEBUG
TEST(BYTESTREAM, INVALID_CONSTRUCTOR)
{
    auto buffer = new char[20];
    memset(buffer, 0, 20);
    EXPECT_DEATH(ByteStream(nullptr, 20), "Assertion failed*");
    EXPECT_DEATH(ByteStream(buffer, 0), "Assertion failed*");
    EXPECT_DEATH(ByteStream(nullptr, 20, true), "Assertion failed*");
    EXPECT_DEATH(ByteStream(buffer, 0, true), "Assertion failed*");
    delete[] buffer;
}
#endif

TEST(BYTESTREAM, TO_STREAM)
{
#ifdef _DEBUG
    {
        char buffer[] = { 'a', 'b', 'c', 'd' };

        BinarySerializer serializer;
        for(int i = 0; i < sizeof(buffer) / sizeof(char); ++i)
        {
            serializer.writeT<char>(buffer[i]);
        }

        EXPECT_DEATH(serializer.toStream(), "Assertion failed*"); //no position reset, nothing to write to stream
    }
#endif

    {
        char buffer[] = { 'a', 'b', 'c', 'd' };

        BinarySerializer serializer;
        for(int i = 0; i < sizeof(buffer) / sizeof(char); ++i)
        {
            serializer.writeT<char>(buffer[i]);
        }

        serializer.resetPosition();
        std::shared_ptr<ByteStream> stream;
        ASSERT_NO_THROW(stream = serializer.toStream());
        ASSERT_EQ(sizeof(buffer) / sizeof(char), (size_t)stream->size());

        for(stream_size_t i = 0; i < serializer.size(); ++i)
        {
            EXPECT_EQ(buffer[i], stream->buffer()[i]);
        }
    }

    {
        char buffer[] = { 'a', 'b', 'c', 'd' };

        BinarySerializer serializer;
        for(int i = 0; i < sizeof(buffer) / sizeof(char); ++i)
        {
            serializer.writeT<char>(buffer[i]);
        }

        std::shared_ptr<ByteStream> stream;
        ASSERT_NO_THROW(stream = serializer.transferToStream()); //no reset required, transfer grabs everything
        ASSERT_EQ(sizeof(buffer) / sizeof(char), (size_t)stream->size());

        for(stream_size_t i = 0; i < serializer.size(); ++i)
        {
            EXPECT_EQ(buffer[i], stream->buffer()[i]);
        }
    }

    {
        char buffer[] = { 'a', 'b', 'c', 'd' };

        BinarySerializer serializer;
        for(int i = 0; i < sizeof(buffer) / sizeof(char); ++i)
        {
            serializer.writeT<char>(buffer[i]);
        }

        std::shared_ptr<ByteStream> stream;
        ASSERT_NO_THROW(stream = serializer.transferToStream());
        ASSERT_EQ(sizeof(buffer) / sizeof(char), (size_t)stream->size());

        for(stream_size_t i = 0; i < serializer.size(); ++i)
        {
            EXPECT_EQ(buffer[i], stream->buffer()[i]);
        }
    }
}

TEST(BYTESTREAM, APPEND)
{
    BinarySerializer serializerA, serializerB;
    serializerA.writeString("First part of ");
    serializerB.writeString("the string written");

    std::shared_ptr<ByteStream> stream;
    EXPECT_NO_THROW(stream = serializerA.transferToStream());
    EXPECT_NO_THROW(stream = stream->append(serializerB.transferToStream()));

    BinarySerializer serializer(stream->buffer(), stream->size());
    std::string partA, partB;
    EXPECT_NO_THROW(serializer.readString(partA));
    EXPECT_NO_THROW(serializer.readString(partB));

    EXPECT_STREQ("First part of ", partA.c_str());
    EXPECT_STREQ("the string written", partB.c_str());
}

TEST(BYTESTREAM, EOF_FUNCTIONS)
{
    BinarySerializer serializer;
    serializer.writeString("test string");

    std::shared_ptr<ByteStream> stream;
    EXPECT_NO_THROW(stream = serializer.transferToStream());

    EXPECT_NO_THROW(stream->appendEof());

    EXPECT_TRUE(stream->hasEof());

    BinarySerializer outSerializer(stream->buffer(), stream->size());
    std::string result;

    EXPECT_NO_THROW(outSerializer.readString(result));
    EXPECT_STREQ("test string", result.c_str());

    int eof;
    EXPECT_NO_THROW(outSerializer.readT<int>(eof));
    EXPECT_EQ(std::ios_base::eofbit, eof);
}

