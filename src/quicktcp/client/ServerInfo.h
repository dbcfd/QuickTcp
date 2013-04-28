#pragma once

#include "quicktcp/client/Platform.h"

#include <functional>

#include <uv.h>

namespace quicktcp {
namespace client {

//------------------------------------------------------------------------------
class CLIENT_API ServerInfo {
public:
   ServerInfo(const std::string& serverAddress, const unsigned int port);

   inline const std::string& address() const;
   inline const unsigned int port() const;

   void resolveAddress(uv_loop_t& loop, std::function<void(int, struct addrinfo*)> onResolution);

private:
   std::string mServerAddress;
   unsigned int mPort;
   uv_getaddrinfo_t mResolver;
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