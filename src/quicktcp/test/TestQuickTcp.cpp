#include "quicktcp/QuickTcp.h"
#include "client/IClient.h"
#include "server/IServer.h"
#include "server/IResponder.h"

#include "async/AsyncResult.h"
#include "workers/Manager.h"
#include "utilities/ByteStream.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp;

class Responder : public server::IResponder
{
public:
    Responder() : server::IResponder(), wasError(false), hadDisconnect(false)
    {

    }
    virtual bool authenticateConnection(std::shared_ptr<utilities::ByteStream> stream)
    {
        return true;
    }
    virtual async_cpp::async::AsyncResult respond(std::shared_ptr<utilities::ByteStream> stream) 
    {
        return async_cpp::async::AsyncResult(stream);
    }
    virtual void handleErrorSendingResponse(async_cpp::async::AsyncResult& result) 
    {
        wasError = true;
    }
    virtual void handleConnectionClosed()
    {
        hadDisconnect = true;
    }
    bool wasError;
    bool hadDisconnect;
};

char MESSAGE[] =  "This is the message";

class QuickTcpTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        manager = std::shared_ptr<async_cpp::workers::IManager>(new async_cpp::workers::Manager(1));
        responder = std::shared_ptr<Responder>(new Responder());
        processFunc = [](std::shared_ptr<utilities::ByteStream> stream) -> async_cpp::async::AsyncResult {
            return async_cpp::async::AsyncResult(stream);
        };
        stream = std::shared_ptr<utilities::ByteStream>(new utilities::ByteStream(MESSAGE, sizeof(MESSAGE)));
    }

    virtual void TearDown()
    {
        manager->shutdown();
        //give windows time to reuse our socket
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::shared_ptr<async_cpp::workers::IManager> manager;
    std::shared_ptr<Responder> responder;
    std::shared_ptr<utilities::ByteStream> stream;
    std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processFunc;
};

TEST_F(QuickTcpTest, SERVER_CONSTRUCTOR)
{
    
    std::shared_ptr<server::IServer> server;
    ASSERT_NO_THROW(server = CreateServer(server::ServerInfo(4765, 5, 5, 2000), manager, responder));
    std::thread serverThread([server]()->void {
        server->waitForEvents();
    });
    //let our thread fire up
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_NO_THROW(server->shutdown());
    serverThread.join();
    ASSERT_NO_THROW(server.reset());
}

TEST_F(QuickTcpTest, SERVER_CLIENT_CONSTRUCTOR)
{
    {
        std::shared_ptr<server::IServer> server;
        ASSERT_NO_THROW(server = CreateServer(server::ServerInfo(4765, 5, 5, 2000), manager, responder));
        std::shared_ptr<client::IClient> client;
        //server accepts the connection, but won't send/receive data
        ASSERT_NO_THROW(client = CreateClient(client::ServerInfo("localhost", 4765), std::shared_ptr<utilities::ByteStream>(), 2000, processFunc));
        ASSERT_NO_THROW(client.reset());
        ASSERT_NO_THROW(server.reset());
    }

    {
        std::shared_ptr<server::IServer> server;
        ASSERT_NO_THROW(server = CreateServer(server::ServerInfo(4765, 5, 5, 2000), manager, responder));
        std::thread serverThread([server]()->void {
            server->waitForEvents();
        });
        //let our thread fire up
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::shared_ptr<client::IClient> client;
        ASSERT_NO_THROW(client = CreateClient(client::ServerInfo("localhost", 4765), std::shared_ptr<utilities::ByteStream>(), 2000, processFunc));
        ASSERT_NO_THROW(client.reset());
        ASSERT_NO_THROW(server->shutdown());
        serverThread.join();
        ASSERT_TRUE(responder->hadDisconnect);
        ASSERT_NO_THROW(server.reset());
    }
}

TEST_F(QuickTcpTest, SERVER_CLIENT_COMMUNICATION)
{
    std::shared_ptr<server::IServer> server;
    ASSERT_NO_THROW(server = CreateServer(server::ServerInfo(4765, 5, 5, 2000), manager, responder));
    std::thread serverThread([server]()->void {
        server->waitForEvents();
    });
    //let our thread fire up
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::shared_ptr<client::IClient> client;
    ASSERT_NO_THROW(client = CreateClient(client::ServerInfo("localhost", 4765), std::shared_ptr<utilities::ByteStream>(), 2000, processFunc));
    auto result = client->request(stream).get();
    std::shared_ptr<utilities::ByteStream> resultStream;
    ASSERT_NO_THROW(resultStream = result.throwOrAs<utilities::ByteStream>());
    ASSERT_TRUE(resultStream);
    ASSERT_STREQ(MESSAGE, (char*)resultStream->buffer());
    ASSERT_NO_THROW(server->shutdown());
    serverThread.join();
    ASSERT_NO_THROW(client.reset());
    ASSERT_NO_THROW(server.reset());
}