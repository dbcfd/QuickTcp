#include "quicktcp/os/windows/server/Server.h"
#include "quicktcp/os/windows/server/Winsock2.h"

#include "async/AsyncResult.h"

#include "workers/Manager.h"

#include "quicktcp/server/IResponder.h"
#include "quicktcp/server/ServerInfo.h"

#include "quicktcp/utilities/ByteStream.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

#include <functional>
#include <iostream>

using namespace quicktcp;

class MockClient
{
public:
    MockClient(const std::chrono::system_clock::duration& dur) : couldConnect(false), waitDuration(dur)
    {
        WSAData wsaData;
        int iResult;
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    }

    void connect(const std::string& port, std::function<void(SOCKET)> afterConnection)
    {
        //define where we're connecting to
        struct addrinfo* results = nullptr, hints;
        ZeroMemory(&hints, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        //get the address information for the host
        int iResult = getaddrinfo("127.0.0.1", port.c_str(), &hints, &results);

        if(0 != iResult)
        {
            throw(std::runtime_error("getaddrinfo failure"));
        }

        if(nullptr == results)
        {
            throw(std::runtime_error("No server"));
        }

        SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, 0);

        char sendData[] = "Send data";
        char recvData[200];
        WSABUF dataBuffer;
        dataBuffer.buf = sendData;
        dataBuffer.len = (ULONG)strlen(sendData);
        WSABUF recvBuffer;
        recvBuffer.buf = recvData;
        recvBuffer.len = 200;

        if(SOCKET_ERROR == WSAConnect(socket, results->ai_addr, (int) results->ai_addrlen, &dataBuffer, &recvBuffer, 0, 0))
        {
            int err = WSAGetLastError();
            throw(std::runtime_error("could not connect"));
        }
        DWORD timeout = 30;
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(DWORD));
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(DWORD));

        freeaddrinfo(results);

        std::this_thread::sleep_for(waitDuration);

        couldConnect = true;

        afterConnection(socket);

        closesocket(socket);

        std::this_thread::sleep_for(waitDuration);
    }

    void sendToServer(const std::string& port, std::function<void(SOCKET)> afterSend)
    {
        connect(port, [this, afterSend](SOCKET socket)->void {
            char buffer[] = "Data sent to server";
            auto len = (int)strlen(buffer);
            int flags = 0;
            if(SOCKET_ERROR == send(socket, buffer, len, flags))
            {
                throw(std::runtime_error("send failed"));
            }
            std::this_thread::sleep_for(waitDuration); //wait for our send to actually go
            afterSend(socket);
        } );
    }

    void sendAndReceive(const std::string& port)
    {
        memset(recvBuffer, 0, sizeof(recvBuffer));
        sendToServer(port, [this](SOCKET socket)->void {
            int flags = 0;
            if(SOCKET_ERROR == recv(socket, recvBuffer, sizeof(recvBuffer), flags))
            {
                throw(std::runtime_error("recv failed"));
            }
        } );
    }

    bool couldConnect;
    std::chrono::system_clock::duration waitDuration;
    char recvBuffer[200];
};

class Responder : public server::IResponder
{
public:
    Responder() : clientDisconnected(false), attemptedResponse(false), hadResponseError(false) {}

    virtual bool authenticateConnection(std::shared_ptr<utilities::ByteStream> stream)
    {
        return true;
    }

    virtual async_cpp::async::AsyncResult respond(std::shared_ptr<utilities::ByteStream> stream)
    {
        attemptedResponse = true;
        std::string result("response from server");
        stream = std::make_shared<utilities::ByteStream>((void*)&result[0], result.size());
        return async_cpp::async::AsyncResult(stream);
    }

    virtual void handleConnectionClosed()
    {
        clientDisconnected = true;
    }

    virtual void handleErrorSendingResponse(async_cpp::async::AsyncResult& result)
    {
        hadResponseError = true;
    }

    bool clientDisconnected;
    bool attemptedResponse;
    bool hadResponseError;
    std::promise<async_cpp::async::AsyncResult> promise;
};

class ServerTest : public testing::Test
{
public:
    ServerTest() : serverInfo(4569, 5, 5, 2000)
    {

    }

    virtual void SetUp()
    {
        manager = std::make_shared<async_cpp::workers::Manager>(5);
        port = "4569";
        responder = std::make_shared<Responder>();
    }

    virtual void TearDown()
    {
        manager->shutdown();
    }

    std::shared_ptr<Responder> responder;
    std::shared_ptr<async_cpp::workers::IManager> manager;
    std::string port;
    server::ServerInfo serverInfo;
};

using namespace quicktcp::os::windows::server;

TEST_F(ServerTest, CONSTRUCTOR)
{
    ASSERT_NO_THROW(Server(serverInfo, manager, responder));
}

TEST_F(ServerTest, ACCEPT_CONNECTION)
{
    Server server(serverInfo, manager, responder);

    std::thread thread([&server]()->void {
        server.waitForEvents();
    } );

    MockClient client(std::chrono::milliseconds(10));

    EXPECT_NO_THROW(client.connect(port, [](SOCKET) {}));

    EXPECT_TRUE(client.couldConnect);

    EXPECT_NO_THROW(server.shutdown());

    EXPECT_TRUE(responder->clientDisconnected);

    thread.join();
}

TEST_F(ServerTest, ACCEPT_CONNECTION_SEND)
{
    Server server(serverInfo, manager, responder);

    std::thread thread([&server]()->void {
        server.waitForEvents();
    } );

    MockClient client(std::chrono::milliseconds(10));

    EXPECT_NO_THROW(client.sendToServer(port, [&client](SOCKET) {} ) );

    EXPECT_TRUE(client.couldConnect);

    EXPECT_NO_THROW(server.shutdown());

    EXPECT_TRUE(responder->clientDisconnected);
    EXPECT_TRUE(responder->attemptedResponse);

    thread.join();
}

TEST_F(ServerTest, ACCEPT_CONNECTION_SEND_RECEIVE)
{
    Server server(serverInfo, manager, responder);

    std::thread thread([&server]()->void {
        server.waitForEvents();
    } );

    MockClient client(std::chrono::milliseconds(10));

    EXPECT_NO_THROW(client.sendAndReceive(port));

    EXPECT_TRUE(client.couldConnect);

    EXPECT_TRUE(responder->clientDisconnected);
    EXPECT_TRUE(responder->attemptedResponse);

    EXPECT_NO_THROW(server.shutdown());

    EXPECT_STREQ("response from server", client.recvBuffer);

    thread.join();
}
