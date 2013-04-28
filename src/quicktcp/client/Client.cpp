#include "quicktcp/client/Client.h"
#include "quicktcp/client/IProcessor.h"
#include "quicktcp/utilities/ByteStream.h"

#include <async/AsyncResult.h>

#include <assert.h>

namespace quicktcp {
namespace client {

//Data structures for c functions
//------------------------------------------------------------------------------
Client::ConnectRequest::ConnectRequest(std::shared_ptr<Client> _client) 
    : client(_client)
{

}

//------------------------------------------------------------------------------
Client::WriteRequest::WriteRequest(std::shared_ptr<Client> _client)
    : client(_client)
{

}

//------------------------------------------------------------------------------
Client::ReadRequest::ReadRequest(std::shared_ptr<Client> _client, std::promise<async_cpp::async::AsyncResult>& _promise)
    : promise(std::move(_promise)), client(_client), buffer(nullptr)
{

}

//------------------------------------------------------------------------------
Client::ProcessRequest::ProcessRequest(std::shared_ptr<Client> _client, 
                                       std::promise<async_cpp::async::AsyncResult>& _promise,
                                       std::shared_ptr<utilities::ByteStream> _stream)
                                       : client(_client), promise(std::move(_promise)), stream(_stream)
{

}

//------------------------------------------------------------------------------
Client::Client(uv_loop_t& loop,
               const ServerInfo& info, 
               std::shared_ptr<utilities::ByteStream> authentication)
    : mInfo(info), mAuthentication(authentication), mIsConnected(false), mLoop(loop)
{
    uv_tcp_init(&loop, &mSocket);

    struct sockaddr_in dest = uv_ip4_addr(info.address().c_str(), info.port());

    auto connection = new ConnectRequest(shared_from_this());
    uv_tcp_connect((uv_connect_t*)connection, &mSocket, dest, &ClientConnectCallback);
}

//------------------------------------------------------------------------------
Client::~Client()
{
    if(mIsConnected)
    {
        disconnect();
    }
}

//------------------------------------------------------------------------------
void Client::disconnect()
{
    mIsConnected = false;
    uv_close((uv_handle_t*)&mSocket, nullptr);
}

//------------------------------------------------------------------------------
void ClientConnectCallback(uv_connect_t* req, int status) { //for c function pointer
    auto connectData = (Client::ConnectRequest*)req;
    connectData->client->onConnect(status);
    delete connectData;
}

void Client::onConnect(int status)
{
    if(-1 != status)
    {
        mIsConnected = true;
        if(mAuthentication)
        {
            //send authentication
        }
        mConnectedSignal.notify_all();
    }
}

//------------------------------------------------------------------------------
void Client::waitForConnection()
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    mConnectedSignal.wait(lock, [this]()->bool {
        return mIsConnected;
    } );
}

//------------------------------------------------------------------------------
uv_buf_t ClientAllocationCallback(uv_handle_t* handle, size_t suggested_size) //for c function pointer
{
    auto readRequest = (Client::ReadRequest*)handle;
    readRequest->buffer = new char[suggested_size];
    uv_buf_t ret;
    ret.base = readRequest->buffer;
    ret.len = (ULONG)suggested_size;
    return ret;
}

//------------------------------------------------------------------------------
void ClientReadCallback(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
    auto readRequest = (Client::ReadRequest*)stream;
    readRequest->client->onReadComplete(nread, buf, *readRequest);
    delete readRequest;
}

void Client::onReadComplete(ssize_t nread, uv_buf_t buf, ReadRequest& req)
{
    switch(nread)
    {
    case -1:
        delete[] req.buffer;
        req.promise.set_value(async_cpp::async::AsyncResult("Client: Error reading from stream"));
        disconnect();
        break;
    case 0:
        //process
        {
            if(mIsConnected)
            {
                auto processReq = new ProcessRequest(shared_from_this(), req.promise, req.readStream);
                uv_queue_work(&mLoop, (uv_work_t*)processReq, ClientProcessRequest, nullptr);
            }
            else
            {
                req.promise.set_value(async_cpp::async::AsyncResult("Client: Client is disconnected"));
            }
        }
        break;
    default:
        if(req.readStream)
        {
            req.readStream->append(std::make_shared<utilities::ByteStream>(buf.base, nread, true));
        }
        else
        {
            req.readStream = std::make_shared<utilities::ByteStream>(buf.base, nread, true);
        }
        break;
    };
}

//------------------------------------------------------------------------------
async_cpp::async::AsyncFuture Client::request(std::shared_ptr<utilities::ByteStream> stream)
{
    async_cpp::async::AsyncFuture result;
    if(mIsConnected)
    {
        auto req = new WriteRequest(shared_from_this());
        req->buf = uv_buf_init(new char[stream->size()], (unsigned int)stream->size());
        memcpy(req->buf.base, stream->buffer(), stream->size());
        uv_write((uv_write_t*)req, (uv_stream_t*)&mSocket, &req->buf, 1, &ClientWriteCallback);
        result = req->promise.get_future();
    }
    else
    {
        result = async_cpp::async::AsyncResult("Client: Client is disconnected").asFulfilledFuture();
    }
    return result;
}

//------------------------------------------------------------------------------
void ClientWriteCallback(uv_write_t* req, int status)
{
    auto writeRequest = (Client::WriteRequest*)req;
    writeRequest->client->onWriteComplete(status, writeRequest->promise);
    delete writeRequest;
}

void Client::onWriteComplete(int status, std::promise<async_cpp::async::AsyncResult>& promise)
{
    //begin read
    if(-1 == status)
    {
        promise.set_value(async_cpp::async::AsyncResult("Client: Error writing to stream"));
    }
    else
    {
        if(mIsConnected)
        {
            auto req = new ReadRequest(shared_from_this(), promise);
            req->stream = (uv_stream_t*)&mSocket;
            uv_read_start((uv_stream_t*)req, &ClientAllocationCallback, &ClientReadCallback);
        }
        else
        {
            promise.set_value(async_cpp::async::AsyncResult("Client: Client is disconnected"));
        }
    }
}

//------------------------------------------------------------------------------
void ClientProcessRequest(uv_work_t* req)
{
    auto procReq = (Client::ProcessRequest*)req;
    procReq->client->process(procReq->stream, procReq->promise);
}

void Client::process(std::shared_ptr<utilities::ByteStream> stream, std::promise<async_cpp::async::AsyncResult>& promise) const
{
    if(mIsConnected)
    {
        if(mProcessor)
        {
            promise.set_value(mProcessor->processResponse(stream));
        }
        else
        {
            promise.set_value(async_cpp::async::AsyncResult(stream));
        }
    }
    else
    {
        promise.set_value(async_cpp::async::AsyncResult("Client: Client is disconnected"));
    }
}

}
}