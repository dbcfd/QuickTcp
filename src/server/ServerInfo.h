#pragma once

#include "server/Platform.h"

namespace quicktcp {
namespace server {

class IServerConnection;

/**
 * Information to create server
 */
class SERVER_API ServerInfo
{
public:
    ServerInfo(const unsigned int port, const unsigned int maxConnections);

    inline const unsigned int port() const;
    inline const unsigned int maxConnections() const;
private:
    unsigned int mPort;
    unsigned int mMaxConnections;
};

//inline implementations
//------------------------------------------------------------------------------
const unsigned int ServerInfo::port() const
{
    return mPort;
}

//------------------------------------------------------------------------------
const unsigned int ServerInfo::maxConnections() const
{
    return mMaxConnections;
}

}
}