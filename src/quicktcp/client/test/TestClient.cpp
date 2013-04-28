#include "quicktcp/client/Client.h"
#include "quicktcp/client/IProcessor.h"
#include "quicktcp/client/ServerInfo.h"
#include "quicktcp/utilities/ByteStream.h"

#include <async/AsyncResult.h>

#include <uv.h>

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

uv_buf_t ConnectionAllocCallback(uv_handle_t* handle, size_t suggested_size)
{
    uv_buf_t ret;
    ret.base = new char[suggested_size];
    ret.len = (ULONG)suggested_size;
    return ret;
}

struct WriteRequest
{
    uv_write_t req;
    uv_buf_t buf;
};

void ConnectionWriteCallback(uv_write_t* req, int)
{
    auto writeReq = (WriteRequest*)req;
    delete[] writeReq->buf.base;
    delete writeReq;
}

void ConnectionReadCallback(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
    auto req = new WriteRequest();
    req->buf.base = buf.base;
    req->buf.len = buf.len;
    uv_write((uv_write_t*)req, stream, &(req->buf), 1, ConnectionWriteCallback);
}

void ServerConnectionCallback(uv_stream_t* server, int status)
{
    if(-1 != status)
    {
        auto client = new uv_tcp_t();
        uv_tcp_init((uv_loop_t*)server->data, client);
        if(0 == uv_accept(server, (uv_stream_t*)client))
        {
            uv_read_start((uv_stream_t*)client, ConnectionAllocCallback, ConnectionReadCallback);
        }
        else
        {
            uv_close((uv_handle_t*)client, nullptr);
            delete client;
        }
    }
}

class MockServer
{
public:
    MockServer(uv_loop_t& _loop, int port) : loop(_loop), buffer(nullptr)
    {
        uv_tcp_init(&loop, &server);

        struct sockaddr_in bind_addr = uv_ip4_addr("127.0.0.1", port);
        uv_tcp_bind(&server, bind_addr);
        server.data = &loop;

        auto r = uv_listen((uv_stream_t*)&server, 50, &ServerConnectionCallback);
    }

    ~MockServer()
    {
        delete[] buffer;
    }

    uv_loop_t& loop;
    uv_tcp_t server;
    char* buffer;
};

TEST(CLIENT_TEST, CLIENT_CONNECT)
{
    auto loop = uv_default_loop();

    auto server = std::make_shared<MockServer>(*loop, 4444);
    auto thread = std::thread([loop]()-> void {
        uv_run(loop, UV_RUN_DEFAULT);
    } );

    auto client = std::make_shared<Client>(*loop, ServerInfo("localhost", 4444), std::shared_ptr<utilities::ByteStream>());

    EXPECT_NO_THROW(client->waitForConnection());
    EXPECT_TRUE(client->isConnected());

    std::future<async_cpp::async::AsyncResult> res;
    EXPECT_NO_THROW(res = client->request(std::shared_ptr<utilities::ByteStream>()));

    EXPECT_NO_THROW(res.get().throwIfError());

    client->disconnect();

    EXPECT_FALSE(client->isConnected());
}

TEST(CLIENT_TEST, CLIENT_REQUEST)
{
    auto loop = uv_default_loop();

    auto server = std::make_shared<MockServer>(*loop, 4444);
    auto thread = std::thread([loop]()-> void {
        uv_run(loop, UV_RUN_DEFAULT);
    } );

    auto client = std::make_shared<Client>(*loop, ServerInfo("localhost", 4444), std::shared_ptr<utilities::ByteStream>());

    EXPECT_NO_THROW(client->waitForConnection());
    EXPECT_TRUE(client->isConnected());

    async_cpp::async::AsyncFuture future;
    std::string message("this is a message");
    auto stream = std::make_shared<utilities::ByteStream>(&message[0], message.size());
    EXPECT_NO_THROW(future = client->request(stream));
    std::shared_ptr<utilities::ByteStream> result;
    ASSERT_NO_THROW(result = future.get().throwOrAs<utilities::ByteStream>());
    std::string resultMessage((std::string::value_type*)result->buffer());

    EXPECT_NO_THROW(client->disconnect());

    EXPECT_FALSE(client->isConnected());
}
