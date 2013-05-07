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
    mSocket.async_receive(mReceiveBuffers, [thisPtr](const boost::system::error_code& ec, std::size_t nbBytes)->void {
        thisPtr->onReadComplete(ec, nbBytes);
    } );
}

//------------------------------------------------------------------------------
void onReadComplete(const boost::system::error_code& ec, std::size_t nbBytes)
{
    if(!ec)
    {
        mReceiveBuffers.clear();
        auto thisPtr = shared_from_this();
        auto data = std::make_shared<utilities::ByteStream>(mBuffer, (stream_size_t)nbBytes);
        mService->post([thisPtr, data, this]()->void {
            auto response = mResponder->respond(data);
            if(!response->hasEof()) response->appendEof();
            mSendBuffers.emplace_back(response->buffer(), response->size());
            mSocket.async_send(mSendBuffers, [thisPtr, this, response](const boost::system::error_code& ec, std::size_t nbBytes)->void {
                onWriteComplete(response, ec, nbBytes);
            } );
        } );
    }
    //else connection will close
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