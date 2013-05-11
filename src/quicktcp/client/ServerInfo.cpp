#include "quicktcp/client/ServerInfo.h"

#include <regex>
#include <string>

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const std::string& serverAddress, const unsigned int port) : mPort(port)
{
    std::regex theReg("((ftp|http(s)?)://)?(\\w[^/]+(/\\w[^/]+)?)", std::regex_constants::icase);
    std::smatch match;
    if(!std::regex_match(serverAddress, match, theReg))
    {
        throw(std::runtime_error("Invalid server address"));
    }
    mServerAddress.assign(match[4]);
}

//------------------------------------------------------------------------------
void ServerInfo::resolveAddress(boost::asio::io_service& ioService, 
                                std::function<void(const boost::system::error_code&, boost::asio::ip::tcp::resolver::iterator)> onResolution)
{
    auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(ioService);
    boost::asio::ip::tcp::resolver::query query(mServerAddress, std::to_string(mPort));
    resolver->async_resolve(query, [resolver, onResolution](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator itr) -> void {
        onResolution(ec, itr);
    } );
}

}
}