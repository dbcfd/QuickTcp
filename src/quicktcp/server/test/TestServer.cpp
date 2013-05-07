#include "quicktcp/server/Server.h"
#include "quicktcp/server/IResponder.h"
#include "quicktcp/server/ServerInfo.h"

#include "quicktcp/utilities/BinarySerializer.h"
#include "quicktcp/utilities/ByteStream.h"

#include <async_cpp/async/ParallelFor.h>
#include <boost/asio.hpp>
#include <thread>
#include <vector>

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp;
using namespace quicktcp::server;

using namespace boost::asio::ip;

class TestResponder : public IResponder {
public:
    TestResponder() : hadConnection(false), hadRequest(false), hadError(false)
    {

    }

    virtual bool authenticateConnection() 
    {
        hadConnection = true;
        return true;
    }

    virtual std::shared_ptr<utilities::ByteStream> respond(std::shared_ptr<utilities::ByteStream> stream)
    {
        hadRequest = true;
        return stream;
    }

    virtual void handleErrorAccepting(const std::string& message) 
    {
        hadError = true;
    }

    virtual void handleErrorSendingResponse(const std::string& message) 
    {
        hadError = true;
    }

    virtual void handleErrorIncompleteSend() 
    {
        hadError = true;
    }

    virtual void handleConnectionClosed()
    {

    }

    bool hadConnection;
    bool hadRequest;
    bool hadError;

};

static const int PORT = 4444;
static const char SENT[] = "this is being sent";

TEST(SERVER_TEST, CONSTRUCTOR)
{
    ASSERT_NO_THROW(Server(std::make_shared<boost::asio::io_service>(), ServerInfo(PORT, 5, 5, 2048), std::make_shared<TestResponder>()));
}

TEST(SERVER_TEST, WAIT_AND_SHUTDOWN)
{
    auto service = std::make_shared<boost::asio::io_service>();
    auto responder = std::make_shared<TestResponder>();
    Server server(service, ServerInfo(PORT, 5, 5, 2048), responder);

    std::thread serverThread([&server]()-> void {
        server.waitForEvents();
    } );

    EXPECT_NO_THROW(server.shutdown());
    EXPECT_NO_THROW(serverThread.join());
}

class MockClient {
public:
    MockClient(boost::asio::io_service& _service)
        : socket(_service), service(_service), receivedResponse(false), sendSuccessful(false)
    {
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), PORT));
        utilities::BinarySerializer serializer(100);
        serializer.writeString(SENT);
        serializer.writeEof();
        auto streamSize = serializer.size();
        sent = std::make_shared<utilities::ByteStream>(serializer.transferBuffer(), streamSize);
    }

    void send()
    {
        std::vector<boost::asio::const_buffer> buffers;
        buffers.emplace_back(sent->buffer(), sent->size());
        auto sentSize = socket.send(buffers);
        sendSuccessful = (sentSize == sent->size());
    }

    void receive()
    {
        auto buffer = new char[200];
        std::vector<boost::asio::mutable_buffer> buffers;
        buffers.emplace_back(buffer, 200);
        auto recvSize = socket.receive(buffers);
        if(0 != recvSize)
        {
            receivedResponse = true;
            received = std::make_shared<utilities::ByteStream>(buffer, (stream_size_t)recvSize, true);
        }
    }

    tcp::socket socket;
    std::shared_ptr<utilities::ByteStream> sent;
    std::shared_ptr<utilities::ByteStream> received;
    boost::asio::io_service& service;
    bool receivedResponse;
    bool sendSuccessful;
};

class ServerTest : public testing::Test {
public:
    virtual void SetUp() final
    {
        service = std::make_shared<boost::asio::io_service>();
        responder = std::make_shared<TestResponder>();
        server = std::make_shared<Server>(service, ServerInfo(PORT, 5, 5, 2048), responder);

        serverThread = std::thread([this]()-> void {
            server->waitForEvents();
        } );
    }

    virtual void TearDown() final
    {
        server->shutdown();
        serverThread.join();
    }

    std::shared_ptr<Server> server;
    std::shared_ptr<boost::asio::io_service> service;
    std::shared_ptr<TestResponder> responder;
    std::thread serverThread;
};

TEST_F(ServerTest, CONNECTION)
{
    ASSERT_NO_THROW(MockClient client(*service));

    //give our server time to work through acceptance and pass to responder
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_TRUE(responder->hadConnection);
}

TEST_F(ServerTest, CONNECTION_REQUEST)
{
    MockClient client(*service);

    //give our server time to work through acceptance and pass to responder
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_TRUE(responder->hadConnection);

    client.send();

    ASSERT_TRUE(client.sendSuccessful);

    //give our server time to work through request and pass to responder
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_TRUE(responder->hadRequest);
}

TEST_F(ServerTest, CONNECTION_REQUEST_RECEIVE)
{
    MockClient client(*service);

    //give our server time to work through acceptance and pass to responder
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_TRUE(responder->hadConnection);

    client.send();

    ASSERT_TRUE(client.sendSuccessful);

    //give our server time to work through request and pass to responder
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_TRUE(responder->hadRequest);

    client.receive();

    ASSERT_TRUE(client.receivedResponse);

    auto recvSize = client.received->size();
    utilities::BinarySerializer serializer(client.received->transferBuffer(), recvSize, true);
    std::string responseFromServer;
    ASSERT_TRUE(serializer.readString(responseFromServer));

    ASSERT_STREQ(SENT, responseFromServer.c_str());
}