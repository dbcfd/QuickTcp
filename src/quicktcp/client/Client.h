#pragma once

#include "quicktcp/client/Platform.h"
#include "quicktcp/client/ServerInfo.h"

#include <async/Async.h>

#include <uv.h>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

class IProcessor;

class CLIENT_API Client : public std::enable_shared_from_this<Client> {
public:
    Client(uv_loop_t& loop,
        const ServerInfo& info, 
        std::shared_ptr<utilities::ByteStream> authentication);
    virtual ~Client();

    async_cpp::async::AsyncFuture request(std::shared_ptr<utilities::ByteStream> stream);
    void disconnect();
    void waitForConnection();

    inline size_t bufferSize() const;
    inline void setProcessor(std::shared_ptr<IProcessor> processor);
    inline bool isConnected() const;

protected:
    struct ConnectRequest {
        ConnectRequest(std::shared_ptr<Client> client);

        uv_connect_t connect;
        std::shared_ptr<Client> client;
    };

    struct WriteRequest {
        WriteRequest(std::shared_ptr<Client> client);

        uv_write_t req;
        uv_buf_t buf;
        std::promise<async_cpp::async::AsyncResult> promise;
        std::shared_ptr<Client> client;
    };

    struct ReadRequest {
        ReadRequest(std::shared_ptr<Client> client, std::promise<async_cpp::async::AsyncResult>& promise);

        uv_stream_t* stream;
        char* buffer;
        std::promise<async_cpp::async::AsyncResult> promise;
        std::shared_ptr<Client> client;
        std::shared_ptr<utilities::ByteStream> readStream;
    };

    struct ProcessRequest {
        ProcessRequest(std::shared_ptr<Client> client, 
            std::promise<async_cpp::async::AsyncResult>& promise,
            std::shared_ptr<utilities::ByteStream> stream);

        uv_work_t work;
        std::shared_ptr<Client> client;
        std::promise<async_cpp::async::AsyncResult> promise;
        std::shared_ptr<utilities::ByteStream> stream;
    };

    void onConnect(int status);
    void onReadComplete(ssize_t nread, uv_buf_t buf, ReadRequest& req);
    void onWriteComplete(int status, std::promise<async_cpp::async::AsyncResult>& promise);
    void process(std::shared_ptr<utilities::ByteStream> stream, std::promise<async_cpp::async::AsyncResult>& promise) const;

    std::shared_ptr<utilities::ByteStream> mAuthentication;
    std::shared_ptr<IProcessor> mProcessor;
    std::thread mThread;
    ServerInfo mInfo;
    uv_tcp_t mSocket;
    uv_loop_t& mLoop;
    bool mIsConnected;
    std::condition_variable mConnectedSignal;

    //c callbacks need friend
    friend void ClientConnectCallback(uv_connect_t* req, int status);
    friend uv_buf_t ClientAllocationCallback(uv_handle_t* handle, size_t suggested_size);
    friend void ClientReadCallback(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
    friend void ClientWriteCallback(uv_write_t* req, int status);
    friend void ClientProcessRequest(uv_work_t* req);
};

//inline implementations
//------------------------------------------------------------------------------
void Client::setProcessor(std::shared_ptr<IProcessor> processor)
{
    mProcessor = processor;
}

//------------------------------------------------------------------------------
bool Client::isConnected() const
{
    return mIsConnected;
}

}
}