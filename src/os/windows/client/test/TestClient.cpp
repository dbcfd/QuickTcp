#include "os/windows/client/Client.h"
#include "os/windows/client/Winsock2.h"

#include "async/AsyncResult.h"

#include "workers/BasicTask.h"
#include "workers/Manager.h"

#include "utilities/ByteStream.h"

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp;

class MockServer {
public:
    MockServer(const std::string& port) : wasError(false)
    {
        //startup winsock
        WSAData wsaData;

        if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw(std::runtime_error("WSAStartup Error"));
        }

        serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, 0);

        //WSA startup successful, create address info
        struct addrinfo* result = nullptr, *ptr = nullptr, hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        int iResult = getaddrinfo("127.0.0.1", port.c_str(), &hints, &result);

        if(iResult != 0)
        {
            WSACleanup();
            throw(std::runtime_error("getaddrinfo Error"));
        }

        //socket creation successful, bind to socket
        iResult = bind(serverSocket, result->ai_addr, (int) result->ai_addrlen);
        freeaddrinfo(result);

        if(SOCKET_ERROR == iResult)
        {
            auto error = std::string("Listen Error: ") + std::to_string(WSAGetLastError());            
            WSACleanup();
            throw(std::runtime_error(error));
        }

        if(SOCKET_ERROR == listen(serverSocket, 2))
        {
            auto error = std::string("Listen Error: ") + std::to_string(WSAGetLastError());            
            closesocket(serverSocket);
            WSACleanup();
            throw(std::runtime_error(error));
        }
    }

    void connect(std::function<void(SOCKET)> postConnect = [](SOCKET){})
    {
        connectedSocket = accept(serverSocket, 0, 0 );
        wasError = (INVALID_SOCKET == connectedSocket);

        if(!wasError)
        {
            postConnect(connectedSocket);
        }
        closesocket(connectedSocket);
    }

    void receive(std::function<void(SOCKET)> postReceive = [](SOCKET){}) 
    {
        connect([this, postReceive](SOCKET sckt)-> void {
            char recvBuff[200];
            memset(recvBuff, 0, sizeof(recvBuff));
            auto bytesReceived = recv(sckt, recvBuff, sizeof(recvBuff), 0);
            if(0 == bytesReceived)
            {
                wasError = true;
            }
            else
            {
                postReceive(sckt);
            }
        } );
    }

    void receiveAndRespond() 
    {
        receive([this](SOCKET sckt)->void {
            char respBuffer[] = "Server Response";
            auto bytesSent = send(sckt, respBuffer, sizeof(respBuffer), 0);
            wasError = (bytesSent != sizeof(respBuffer));
        } );
    }

    ~MockServer()
    {
        closesocket(serverSocket);
        WSACleanup();
    }

    SOCKET connectedSocket;
    SOCKET serverSocket;
    char buffer[200];
    bool wasError;
};

using namespace os::windows::client;

class ClientTest : public testing::Test {
public :
    ClientTest() : serverInfo("127.0.0.1", 4567)
    {

    }

    virtual void SetUp()
    {
        server = std::shared_ptr<MockServer>(new MockServer("4567"));
        processStreamFunc = [](std::shared_ptr<utilities::ByteStream> stream)->async_cpp::async::AsyncResult {
            return async_cpp::async::AsyncResult(stream);
        };
    }

    virtual void TearDown()
    {

    }

    std::shared_ptr<MockServer> server;
    client::ServerInfo serverInfo;
    std::function<async_cpp::async::AsyncResult(std::shared_ptr<utilities::ByteStream>)> processStreamFunc;
};

TEST_F(ClientTest, CONSTRUCTOR)
{
    std::thread thread([this]()->void {
        server->connect();
    } );
    //let server start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_NO_THROW(Client(serverInfo, std::shared_ptr<utilities::ByteStream>(), 2048, processStreamFunc));
    thread.join();
    ASSERT_FALSE(server->wasError);
}

TEST_F(ClientTest, SEND)
{
    {
        bool successfulReceive = false;
        std::thread thread([this, &successfulReceive]()->void {
            server->receive([&successfulReceive](SOCKET) -> void {
                successfulReceive = true;
            } );
        } );
        //let server start
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::shared_ptr<Client> client;
        ASSERT_NO_THROW(client = std::shared_ptr<Client>(new Client(serverInfo, std::shared_ptr<utilities::ByteStream>(), 2048, processStreamFunc)));
        //let client have a chance to see if it triggers a send with no data
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ASSERT_FALSE(successfulReceive);
        client.reset(); //close the client, which should stop the receive
        thread.join();
        ASSERT_TRUE(server->wasError); //no bytes received, server will flag error
        ASSERT_FALSE(successfulReceive);
    }

    {
        bool successfulReceive = false;
        std::thread thread([this, &successfulReceive]()->void {
            server->receive([&successfulReceive](SOCKET) -> void {
                successfulReceive = true;
            } );
        } );
        //let server start
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::shared_ptr<Client> client;
        ASSERT_NO_THROW(client = std::shared_ptr<Client>(new Client(serverInfo, std::shared_ptr<utilities::ByteStream>(), 2048, processStreamFunc)));
        char sendBuf[] = "Sending to server";
        std::shared_ptr<utilities::ByteStream> stream(new utilities::ByteStream(sendBuf, sizeof(sendBuf)));
        ASSERT_NO_THROW(client->request(stream));
        thread.join();
        ASSERT_FALSE(server->wasError);
        ASSERT_TRUE(successfulReceive);
    }
}

TEST_F(ClientTest, SEND_RECEIVE)
{
    std::thread thread([this]()->void {
        server->receiveAndRespond();
    } );
    //let server start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::shared_ptr<Client> client;
    ASSERT_NO_THROW(client = std::shared_ptr<Client>(new Client(serverInfo, std::shared_ptr<utilities::ByteStream>(), 2048, processStreamFunc)));
    char sendBuf[] = "Sending to server";
    std::shared_ptr<utilities::ByteStream> stream(new utilities::ByteStream(sendBuf, sizeof(sendBuf)));
    async_cpp::async::AsyncResult result;
    ASSERT_NO_THROW(result = client->request(stream).get());
    ASSERT_FALSE(server->wasError);
    ASSERT_NO_THROW(result.throwIfError());
    auto responseStream = std::static_pointer_cast<utilities::ByteStream>(result.result());
    ASSERT_STREQ("Server Response", (const char*)responseStream->buffer());
    thread.join();
}