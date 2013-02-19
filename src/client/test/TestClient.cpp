#include "client/IClient.h"
#include "client/ServerInfo.h"

#include "workers/Manager.h"

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
    MockClient(std::shared_ptr<workers::Manager> manager, const ServerInfo& info, std::function<void(std::shared_ptr<utilities::ByteStream>)> onReceive)
        : IClient(manager, info, onReceive), isConnected(false)
    {
        isConnected = true;
    }

    virtual void disconnect()
    {
        isConnected = false;
    }

    bool isConnected;

protected:
    virtual void performSend(std::unique_ptr<std::promise<SendResult>> responsePromise, std::shared_ptr<utilities::ByteStream> stream)
    {
        responsePromise->set_value(SendResult());
    }
};


TEST(CLIENT_TEST, CLIENT)
{
    std::shared_ptr<workers::Manager> manager(new workers::Manager(1));
    std::shared_ptr<MockClient> client(new MockClient(manager, ServerInfo("localhost", 8080), [](std::shared_ptr<utilities::ByteStream>){}));

    EXPECT_TRUE(client->isConnected);

    std::future<IClient::SendResult> res;

    EXPECT_NO_THROW(res = client->send(std::shared_ptr<utilities::ByteStream>()));

    EXPECT_NO_THROW(res.get().check());

    client->disconnect();

    EXPECT_FALSE(client->isConnected);
}
