#include "quicktcp/server/Server.h"
#include "quicktcp/server/IResponder.h"

#include "quicktcp/utilities/ByteStream.h"

#include <boost/asio.hpp>

namespace quicktcp {
namespace server {

using namespace boost::asio::ip;

//BEGIN SERVER::TCPCONNECTION
class Server::TcpConnection : public std::enable_shared_from_this<Server::TcpConnection> {
public:

//------------------------------------------------------------------------------
TcpConnection(std::shared_ptr<boost::asio::io_service> service, 
              std::shared_ptr<IResponder> responder,
              const size_t bufferSize)
              : mService(service), mResponder(responder), mSocket(*service), mBufferSize(bufferSize)
{
    mBuffer = new char[bufferSize];
    mReceiveBuffers.reserve(1);
    mSendBuffers.reserve(1);
}

//------------------------------------------------------------------------------
virtual ~TcpConnection()
{
    delete[] mBuffer;
}

//------------------------------------------------------------------------------
void startRead()
{
    auto thisPtr = shared_from_this();
    mReceiveBuffers.emplace_back(mBuffer, mBufferSize);
    boost::asio::async_read(mSocket, mReceiveBuffers, boost::asio::transfer_at_least(1), 
        [thisPtr, this](const boost::system::error_code& ec, std::size_t nbBytes)->void 
        {
            onReadComplete(ec, nbBytes);
        } 
    );
}

//------------------------------------------------------------------------------
void transferBuffer(std::size_t nbBytes)
{
    auto transferred = std::make_shared<utilities::ByteStream>(mBuffer, (stream_size_t)nbBytes, true);
    mBuffer = new char[mBufferSize];
    if(mStream)
    {
        mStream = mStream->append(transferred);
    }
    else
    {
        mStream = transferred;
    }
}

//------------------------------------------------------------------------------
void onReadComplete(const boost::system::error_code& ec, std::size_t nbBytes)
{
    //if we didn't have an error, or the error was eof, than the read was okay
    if(!ec || boost::asio::error::eof == ec.value())
    {
        //transfer bytes if some were received
        if(nbBytes > 0)
        {
            transferBuffer(nbBytes);
        }
        if(mStream)
        {
            if(mStream->hasEof() || boost::asio::error::eof == ec.value())
            {
                //done receiving
                mReceiveBuffers.clear();

                auto thisPtr = shared_from_this();
                auto request = mStream;
                mStream.reset();
                mService->post([thisPtr, request, this]()->void {
                    auto response = mResponder->respond(request);
                    if(!response->hasEof()) response->appendEof();
                    mSendBuffers.emplace_back(response->buffer(), response->size());
                    mSocket.async_send(mSendBuffers, [thisPtr, this, response](const boost::system::error_code& ec, std::size_t nbBytes)->void {
                        onWriteComplete(response, ec, nbBytes);
                    } );
                } );
            }
            else
            {
                //more to read
                mReceiveBuffers.clear();
                mReceiveBuffers.emplace_back(mBuffer, mBufferSize);
                auto thisPtr = shared_from_this();
                boost::asio::async_read(mSocket, mReceiveBuffers, boost::asio::transfer_at_least(1), 
                    [thisPtr, this](const boost::system::error_code& ec, std::size_t nbBytes)->void 
                    {
                        onReadComplete(ec, nbBytes);
                    } 
                );
            }
        }
    }
    //else connection will close due to shared_ptr clean up 
}

//------------------------------------------------------------------------------
void onWriteComplete(std::shared_ptr<utilities::ByteStream> response, const boost::system::error_code& ec, std::size_t nbBytes)
{
    mSendBuffers.clear();
    if(!ec)
    {
        if(response->size() != nbBytes)
        {
            //incomplete send
            mResponder->handleErrorIncompleteSend();
        }
        startRead();
    }
    //else connection will close
}

//------------------------------------------------------------------------------
tcp::socket& getSocket()
{
    return mSocket;
}

private:
    std::shared_ptr<boost::asio::io_service> mService;
    std::shared_ptr<IResponder> mResponder;
    std::shared_ptr<utilities::ByteStream> mStream;
    tcp::socket mSocket;
    std::vector<boost::asio::mutable_buffer> mReceiveBuffers;
    std::vector<boost::asio::const_buffer> mSendBuffers;
    char* mBuffer;
    size_t mBufferSize;
};
//END SERVER::TCPCONNECTION

//BEGIN SERVER::TCPSERVER
class Server::TcpServer {
public:

//------------------------------------------------------------------------------
TcpServer(std::shared_ptr<boost::asio::io_service> ioService, 
    const ServerInfo& info, 
    std::shared_ptr<IResponder> responder)
    : mService(ioService), mInfo(info), mResponder(responder), mAcceptor(*ioService, tcp::endpoint(tcp::v4(), info.port())), mAccepting(true)
{
    startAccept();
}

//------------------------------------------------------------------------------
void startAccept()
{
    auto connection = std::make_shared<TcpConnection>(mService, mResponder, mInfo.bufferSize());
    mAcceptor.async_accept(connection->getSocket(), [connection, this](const boost::system::error_code& ec) -> void {
        if(!ec)
        {
            if(mResponder->authenticateConnection())
            {
                connection->startRead();
            }
            //else not authenticated, connection will fall out of scope
            startAccept();
        }
        else
        {
            mResponder->handleErrorAccepting(ec.message());
        }
    } );
}

//------------------------------------------------------------------------------
void stopAccepting()
{
    mAccepting = false;
    mAcceptor.cancel();
    mAcceptor.close();
}

private:
    std::shared_ptr<boost::asio::io_service> mService;
    std::shared_ptr<IResponder> mResponder;
    tcp::acceptor mAcceptor;
    bool mAccepting;
    ServerInfo mInfo;
};
//END SERVER::TCPSERVER

//------------------------------------------------------------------------------
Server::Server(std::shared_ptr<boost::asio::io_service> ioService, 
        const quicktcp::server::ServerInfo& info, 
        std::shared_ptr<IResponder> responder) 
        : mServer(new TcpServer(ioService, info, responder)), mService(ioService)
{
    assert(ioService);
    mRunning.store(true);
}

//------------------------------------------------------------------------------
Server::~Server()
{
    shutdown();
}

//------------------------------------------------------------------------------
void Server::shutdown()
{
    bool wasRunning = mRunning.exchange(false);
    if(wasRunning)
    {
        mServer->stopAccepting();
    }
}

//------------------------------------------------------------------------------
void Server::waitForEvents()
{
    mService->run();
}

}
}