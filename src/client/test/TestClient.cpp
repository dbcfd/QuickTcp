#include "client/IClient.h"
#include "client/ServerInfo.h"

#include "async/AsyncResult.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp;
using namespace quicktcp::client;

TEST(CLIENT_TEST, SERVER_INFO)
{
    EXPECT_NO_THROW(ServerInfo("http://localhost", 100));
    EXPECT_NO_THROW(ServerInfo("localhost", 100));
    EXPECT_NO_THROW(ServerInfo("https://somewebsite.com", 100));
    EXPECT_NO_THROW(ServerInfo("ftp://someplace.edu.au/directory", 100));
    EXPECT_THROW(ServerInfo("bad://somewhere", 100), std::runtime_error);

    ServerInfo info("http://localhost.myplace.local", 8080);
    EXPECT_STREQ("localhost.myplace.local", info.address().c_str());
}

class MockClient : public IClient
{
public:
    MockClient(const ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication, 
        const size_t bufferSize)
        : IClient(info, authentication, bufferSize), isConnected(false)
    {
        isConnected = true;
    }

    virtual void disconnect()
    {
        isConnected = false;
    }

    bool isConnected;

    virtual std::future<async_cpp::async::AsyncResult> request(std::shared_ptr<utilities::ByteStream> stream)
    {
        std::promise<async_cpp::async::AsyncResult> promise;
        promise.set_value(async_cpp::async::AsyncResult(std::shared_ptr<void>(new bool(true))));
        return promise.get_future();
    }
};


TEST(CLIENT_TEST, CLIENT)
{
    std::shared_ptr<MockClient> client(new MockClient(ServerInfo("localhost", 8080), std::shared_ptr<utilities::ByteStream>(), 100));

    EXPECT_TRUE(client->isConnected);

    std::future<async_cpp::async::AsyncResult> res;
    EXPECT_NO_THROW(res = client->request(std::shared_ptr<utilities::ByteStream>()));

    EXPECT_NO_THROW(res.get().throwIfError());

    client->disconnect();

    EXPECT_FALSE(client->isConnected);
}
