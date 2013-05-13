#include "quicktcp/client/Client.h"
#include "quicktcp/client/IAuthenticator.h"
#include "quicktcp/client/IProcessor.h"
#include "quicktcp/client/ServerInfo.h"

#include "quicktcp/server/IResponder.h"
#include "quicktcp/server/Server.h"
#include "quicktcp/server/ServerInfo.h"

#include "quicktcp/utilities/BinarySerializer.h"
#include "quicktcp/utilities/ByteStream.h"

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

//Mock up responder for server
class MockResponder : public server::IResponder {
public:
    MockResponder() : hadRequest(false)
    {
    
    }

    virtual bool authenticateConnection() final
    {
        return true;
    }

    virtual std::shared_ptr<utilities::ByteStream> respond(std::shared_ptr<utilities::ByteStream> stream) final
    {
        hadRequest = true;
        return stream;
    }

    virtual void handleErrorAccepting(const std::string& message) final
    {
        
    }

    virtual void handleErrorSendingResponse(const std::string& message) final
    {
        
    }

    virtual void handleErrorIncompleteSend() final
    {
        
    }

    virtual void handleConnectionClosed() final
    {

    }
    bool hadRequest;
};

//mock up a processor
class MockProcessor : public IProcessor<> {
public:
    MockProcessor() : disconnected(false), errorOccurred(false), processedResponse(false)
    {

    }

    virtual async_cpp::async::AsyncResult<utilities::ByteStream> processResponse(std::shared_ptr<utilities::ByteStream> stream) final
    {
        processedResponse = true;
        return stream;
    }

    virtual void handleDisconnect() final
    {
        disconnected = true;
    }

    virtual void handleErrorResolveAddress(const std::string& message) final
    {
        errorOccurred = true;
    }

    virtual void handleErrorConnect(const std::string& message) final
    {
        errorOccurred = true;
    }

    bool disconnected;
    bool processedResponse;
    bool errorOccurred;
};

//setup our fixture
class ClientTest : public testing::Test {
public:
    virtual void SetUp() final
    {
        utilities::BinarySerializer serializer(50);
        serializer.writeString("This is the request");
        requestStream = serializer.transferToStream();
        processor = std::make_shared<MockProcessor>();
        responder = std::make_shared<MockResponder>();
        service = std::make_shared<boost::asio::io_service>();
        testServer = std::unique_ptr<server::Server>(new server::Server(
            service,
            server::ServerInfo(4444, 50, 10, 2048),
            responder
        ) );
        serverThread = std::thread([this]()->void {
            testServer->waitForEvents();
        } );
    }

    virtual void TearDown() final
    {
        requestStream.reset();
        testServer->shutdown();
        service->stop();
        serverThread.join();
        testServer.reset();
    }

    std::shared_ptr<boost::asio::io_service> service;
    std::unique_ptr<server::Server> testServer;
    std::thread serverThread;
    std::shared_ptr<MockResponder> responder;
    std::shared_ptr<MockProcessor> processor;
    std::shared_ptr<utilities::ByteStream> requestStream;
};

TEST_F(ClientTest, CONSTRUCTOR)
{
    ASSERT_NO_THROW(Client<>(service, client::ServerInfo("127.0.0.1", 4444), processor));
}

TEST_F(ClientTest, CONNECTION)
{
    auto client = std::make_shared<Client<>>(service, client::ServerInfo("127.0.0.1", 4444), processor);
    ASSERT_NO_THROW(client->connect());
    client->waitForConnection();
    ASSERT_TRUE(client->isConnected());
    ASSERT_NO_THROW(client->disconnect());
    ASSERT_NO_THROW(client.reset());
}

TEST_F(ClientTest, REQUEST)
{
    auto client = std::make_shared<Client<>>(service, client::ServerInfo("127.0.0.1", 4444), processor);
    ASSERT_NO_THROW(client->connect());
    client->waitForConnection();
    ASSERT_TRUE(client->isConnected());
    std::future<async_cpp::async::AsyncResult<utilities::ByteStream>> future;
    ASSERT_NO_THROW(future = client->request(requestStream));
    future.wait();
    EXPECT_TRUE(responder->hadRequest);
    EXPECT_TRUE(processor->processedResponse);
    std::shared_ptr<utilities::ByteStream> responseStream;
    ASSERT_NO_THROW(responseStream = future.get().throwOrGet());
    utilities::BinarySerializer serializer(responseStream->buffer(), responseStream->size());
    std::string result;
    EXPECT_TRUE(serializer.readString(result));
    EXPECT_STREQ("This is the request", result.c_str());
    ASSERT_NO_THROW(client->disconnect());
    ASSERT_NO_THROW(client.reset());
}