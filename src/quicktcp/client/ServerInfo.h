#pragma once

#include "quicktcp/client/Platform.h"

#include <boost/asio.hpp>
#include <functional>

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
class CLIENT_API ServerInfo {
public:
   ServerInfo(const std::string& serverAddress, const unsigned int port);

   inline const std::string& address() const;
   inline const unsigned int port() const;

   void resolveAddress(boost::asio::io_service& ioService, std::function<void(const boost::system::error_code&, boost::asio::ip::tcp::resolver::iterator)> onResolution);

private:
   std::string mServerAddress;
   unsigned int mPort;
};

//inline implementations
//------------------------------------------------------------------------------
const std::string& ServerInfo::address() const
{
    return mServerAddress;
}

//------------------------------------------------------------------------------
const unsigned int ServerInfo::port() const
{
    return mPort;
}

}
}