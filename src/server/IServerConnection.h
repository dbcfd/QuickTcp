#pragma once

#include "server/Platform.h"
#include "server/IServer.h"

#include <future>
#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class IServer;

class SERVER_API IServerConnection {
public:
    class SERVER_API Response
    {
    public:
        Response(std::shared_ptr<utilities::ByteStream> stream);
        Response(const std::string& error);

        inline const std::string& error() const;
        inline std::shared_ptr<utilities::ByteStream> stream() const;

    private:
        std::shared_ptr<utilities::ByteStream> mStream;
        std::string mError;
    };

    class SERVER_API ResponseComplete
    {
    public:
        ResponseComplete(const std::string& error);
        ResponseComplete();

        inline const std::string& error() const;
    private:
        std::string mError;
    };

    IServerConnection(std::shared_ptr<IServer> server);

    std::future<ResponseComplete> respond(std::shared_ptr<utilities::ByteStream> stream);

    inline std::shared_ptr<IServer> server() const;

    virtual void disconnect();

protected:
    virtual Response determineResponse(std::shared_ptr<utilities::ByteStream> stream) = 0;

private:
   std::shared_ptr<IServer> mServer;
};

//inline implementations
//------------------------------------------------------------------------------
const std::string& IServerConnection::Response::error() const
{
    return mError;
}

//------------------------------------------------------------------------------
std::shared_ptr<utilities::ByteStream> IServerConnection::Response::stream() const
{
    return mStream;
}

//------------------------------------------------------------------------------
const std::string& IServerConnection::ResponseComplete::error() const
{
    return mError;
}

//------------------------------------------------------------------------------
std::shared_ptr<IServer> IServerConnection::server() const
{
    return mServer;
}

}
}