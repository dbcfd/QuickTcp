#pragma once

#include "quicktcp/client/Platform.h"
#include "quicktcp/client/IProcessor.h"

#include "quicktcp/utilities/ByteStream.h"

#include <async_cpp/async/AsyncResult.h>
#include <boost/asio/buffer.hpp>
#include <future>
#include <memory>

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
template<class T = utilities::ByteStream>
class PendingRequest {
public:
    PendingRequest(std::shared_ptr<utilities::ByteStream> stream, size_t recvBufferSize);

    const std::vector<boost::asio::const_buffer>& sendBuffers() const;
    const std::vector<boost::asio::mutable_buffer>& recvBuffers() const;
    const bool wasSendValid(const size_t nbBytes) const;
    std::future<async_cpp::async::AsyncResult<T>> getFuture();

    void appendData(const std::size_t nbBytes);
    void complete(std::shared_ptr<IProcessor<T>> processor);
    void fail(const std::string& message);

private:
    std::packaged_task<async_cpp::async::AsyncResult<T>(std::shared_ptr<IProcessor<T>>, std::shared_ptr<utilities::ByteStream>, std::string const*)> mTask;
    std::shared_ptr<utilities::ByteStream> mSentStream;
    std::shared_ptr<utilities::ByteStream> mReceivedStream;
    std::unique_ptr<char[]> mRecvBuffer;
    std::vector<boost::asio::const_buffer> mSendBuffers;
    std::vector<boost::asio::mutable_buffer> mRecvBuffers;
};
//inline implementations
//------------------------------------------------------------------------------
template<class T>
PendingRequest<T>::PendingRequest(std::shared_ptr<utilities::ByteStream> stream, size_t recvBufferSize) : mSentStream(stream)
{
    mTask = std::packaged_task<async_cpp::async::AsyncResult<T>(std::shared_ptr<IProcessor<T>>, std::shared_ptr<utilities::ByteStream>, std::string const*)>(
        [](std::shared_ptr<IProcessor<T>> processor, std::shared_ptr<utilities::ByteStream> serverStream, std::string const* failString) -> async_cpp::async::AsyncResult<T> 
    {
        async_cpp::async::AsyncResult<T> res;
        if(nullptr == failString) 
        {
            if(serverStream->hasEof())
            {
                serverStream->stripEof();
            }
            res = async_cpp::async::AsyncResult<T>(processor->processResponse(serverStream));
        }
        else
        {
            res = async_cpp::async::AsyncResult<T>(*failString);
        }
        return res;
    } 
    );

    mRecvBuffer = std::unique_ptr<char[]>(new char[recvBufferSize]);

    mSendBuffers.emplace_back(stream->buffer(), stream->size());
    mRecvBuffers.emplace_back(mRecvBuffer.get(), recvBufferSize);
}

//------------------------------------------------------------------------------
template<class T>
const std::vector<boost::asio::const_buffer>& PendingRequest<T>::sendBuffers() const
{
    return mSendBuffers;
}

//------------------------------------------------------------------------------
template<class T>
const std::vector<boost::asio::mutable_buffer>& PendingRequest<T>::recvBuffers() const
{
    return mRecvBuffers;
}

//------------------------------------------------------------------------------
template<class T>
const bool PendingRequest<T>::wasSendValid(const size_t nbBytes) const
{
    return (mSentStream->size() == nbBytes);
}

//------------------------------------------------------------------------------
template<class T>
std::future<async_cpp::async::AsyncResult<T>> PendingRequest<T>::getFuture()
{
    return mTask.get_future();
}

//------------------------------------------------------------------------------
template<class T>
void PendingRequest<T>::appendData(const std::size_t nbBytes)
{
    auto stream = std::make_shared<utilities::ByteStream>(mRecvBuffer.get(), (stream_size_t)nbBytes);
    if(mReceivedStream)
    {
        mReceivedStream = mReceivedStream->append(stream);
    }
    else
    {
        mReceivedStream = stream;
    }
}

//------------------------------------------------------------------------------
template<class T>
void PendingRequest<T>::complete(std::shared_ptr<IProcessor<T>> processor)
{
    assert(processor);
    mTask(processor, mReceivedStream, nullptr);
}

//------------------------------------------------------------------------------
template<class T>
void PendingRequest<T>::fail(const std::string& message)
{
    assert(!message.empty());
    mTask(nullptr, nullptr, &message);
}

}
}