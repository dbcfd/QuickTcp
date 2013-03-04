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
            std::stringstream sstr;
            sstr << "listen Error " << WSAGetLastError();
            WSACleanup();
            throw(std::runtime_error(sstr.str()));
        }

        if(SOCKET_ERROR == listen(serverSocket, 2))
        {
            std::stringstream sstr;
            sstr << "listen Error " << WSAGetLastError();
            closesocket(serverSocket);
            WSACleanup();
            throw(std::runtime_error(sstr.str()));
        }

        buffer.buf = (char*)malloc(sizeof(char) * 2000);
        buffer.len = 0;
    }

    void connect()
    {
        thread = std::shared_ptr<std::thread>(new std::thread([this]()-> void {
            auto connectedSocket = accept(serverSocket, 0, 0 );
            wasError = (INVALID_SOCKET == connectedSocket);
            closesocket(connectedSocket);
        } ) );
    }

    void receive(std::function<void(SOCKET)> postReceive = [](SOCKET){}) {
        thread = std::shared_ptr<std::thread>(new std::thread([this, postReceive]()-> void {
            SOCKET connectedSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
            DWORD bytesReceived = 0;
            DWORD flags = 0;
            WSAOVERLAPPED overlap;
            SecureZeroMemory((PVOID) & overlap, sizeof (WSAOVERLAPPED));
            overlap.hEvent = WSACreateEvent();
            wasError = (TRUE == AcceptEx(serverSocket, connectedSocket, 0, 0, sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &bytesReceived, &overlap));
            if(wasError) return;
            setsockopt(connectedSocket, SO_UPDATE_ACCEPT_CONTEXT);
            int rc = WSARecv(connectedSocket, &buffer, 1, &bytesReceived, &flags, &overlap, 0);
            if(rc == SOCKET_ERROR && WSA_IO_PENDING != WSAGetLastError())
            {
                std::stringstream sstr;
                sstr << "recv error:" << WSAGetLastError();
                wasError = true;
            }
            else
            {
                rc = WSAWaitForMultipleEvents(1, &overlap.hEvent, TRUE, WSA_INFINITE, TRUE);
                if(WSA_WAIT_FAILED == rc)
                {
                    std::stringstream sstr;
                    sstr << "wait error:" << WSAGetLastError();
                    wasError = true;
                }

                DWORD flags = 0;
                rc = WSAGetOverlappedResult(connectedSocket, &overlap, &bytesReceived, FALSE, &flags);
                if(0 == rc)
                {
                    std::stringstream sstr;
                    sstr << "get error:" << WSAGetLastError();
                    wasError = true;
                }
                else
                {
                    stream = std::shared_ptr<utilities::ByteStream>(new utilities::ByteStream(buffer.buf, bytesReceived));
                }
            }
            postReceive(connectedSocket);
            closesocket(connectedSocket);
        } ) );
    }

    void receiveAndRespond() 
    {
        receive([this](SOCKET socket)->void {
            char respBuffer[] = "Server Response";
            buffer.buf = respBuffer;
            buffer.len = strlen(respBuffer);
            DWORD flags = 0;
            wasError = (SOCKET_ERROR == WSASend(socket, &buffer, 1, &buffer.len, flags, 0, 0));
        } );
    }

    ~MockServer()
    {
        free(buffer.buf);
        closesocket(serverSocket);
    }

    SOCKET serverSocket;
    WSABUF buffer;
    std::shared_ptr<utilities::ByteStream> stream;
    std::shared_ptr<std::thread> thread;
    bool wasError;
};

class Listener : public client::IClient::IListener {
public:
    Listener() 
    {
        future = promise.get_future();
    }

    virtual void receive(std::shared_ptr<utilities::ByteStream> stream)
    {
        promise.set_value(async_cpp::async::AsyncResult(stream));
    }

    virtual void serverDisconnected()
    {

    }

    std::future<async_cpp::async::AsyncResult> future;
    std::promise<async_cpp::async::AsyncResult> promise;
};

using namespace os::windows::client;

class ClientTest : public testing::Test {
public :
    ClientTest() : serverInfo("127.0.0.1", 4567)
    {

    }

    virtual void SetUp()
    {
        manager = std::shared_ptr<async_cpp::workers::IManager>(new async_cpp::workers::Manager(2));
        server = std::shared_ptr<MockServer>(new MockServer("4567"));
        listener = std::shared_ptr<Listener>(new Listener());
    }

    virtual void TearDown()
    {

    }

    std::shared_ptr<MockServer> server;
    std::shared_ptr<async_cpp::workers::IManager> manager;
    std::shared_ptr<Listener> listener;
    client::ServerInfo serverInfo;
};

TEST_F(ClientTest, CONSTRUCTOR)
{
    server->connect();
    std::shared_ptr<Client> client;
    ASSERT_NO_THROW(Client(manager, serverInfo, listener, 2048));
    server->thread->join();
    ASSERT_FALSE(server->wasError);
}

TEST_F(ClientTest, SEND)
{
    std::shared_ptr<Client> client;
    server->receive();
    ASSERT_NO_THROW(client = std::shared_ptr<Client>(new Client(manager, serverInfo, listener, 2048)));
    std::string send("send to server");
    std::shared_ptr<utilities::ByteStream> stream(new utilities::ByteStream(&send[0], send.size()));
    ASSERT_FALSE(client->send(stream).get().wasError());
    //client->waitForEvents();
    server->thread->join();
    ASSERT_FALSE(server->wasError);
    ASSERT_STREQ(send.c_str(), (char*)server->stream->buffer());
}

TEST_F(ClientTest, SEND_RECEIVE)
{
    std::shared_ptr<Client> client;
    server->receiveAndRespond();
    ASSERT_NO_THROW(client = std::shared_ptr<Client>(new Client(manager, serverInfo, listener, 2048)));
    std::string send("send to server");
    std::shared_ptr<utilities::ByteStream> stream(new utilities::ByteStream(&send[0], send.size()));
    ASSERT_FALSE(client->send(stream).get().wasError());
    auto result = listener->future.get();
    ASSERT_FALSE(result.wasError());
    ASSERT_STREQ("Server Response", (char*)std::static_pointer_cast<utilities::ByteStream>(result.result())->buffer());
}