#include "client/ServerInfo.h"

#include <regex>

namespace quicktcp {
namespace client {

ServerInfo::ServerInfo(const std::string& serverAddress, const unsigned int port) : mPort(port)
{
    std::regex theReg("((ftp|http(s)?)://)?(\\w[\\w\\.]+(/\\w[\\w\\.]+)?)", std::regex_constants::icase);
    std::smatch match;
    if(!std::regex_match(serverAddress, match, theReg))
    {
        throw(std::runtime_error("Invalid server address"));
    }
    mServerAddress.assign(match[4]);
}

}
}