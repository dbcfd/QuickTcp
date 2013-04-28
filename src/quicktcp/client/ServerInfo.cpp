#include "quicktcp/client/ServerInfo.h"

#include <regex>
#include <string>
#include <uv.h>

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void ServerInfo::resolveAddress(uv_loop_t& loop, std::function<void(int, struct addrinfo*)> onResolution)
{
    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_cb* cb = std::function<void(uv_getaddrinfo_t*, int, struct addrinfo*)>(
        [this, onResolution](uv_getaddrinfo_t* resolver, int status, struct addrinfo* res)->void
        {
            onResolution(status, res);
        }
    ).target<uv_getaddrinfo_cb>();

    if(uv_getaddrinfo(&loop, &mResolver, *cb, mServerAddress.c_str(), std::to_string(mPort).c_str(), &hints))
    {
        throw(std::runtime_error(std::string("ServerInfo: getaddrinfo error ") + uv_err_name(uv_last_error(&loop))));
    }
}

}
}