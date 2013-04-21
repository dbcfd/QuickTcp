#pragma once

#include "quicktcp/server/Platform.h"

namespace quicktcp {
namespace server {

class IServerConnection;

/**
 * Information to create server
 */
class SERVER_API ServerInfo
{
public:
    ServerInfo(const unsigned int port, const size_t maxConnections, const size_t maxBacklog, const size_t bufferSize);

    inline const unsigned int port() const;
    inline const size_t maxConnections() const;
    inline const size_t maxBacklog() const;
    inline const size_t bufferSize() const;
private:
    unsigned int mPort;
    size_t mMaxConnections;
    size_t mMaxBacklog;
    size_t mBufferSize;
};

//inline implementations
//------------------------------------------------------------------------------
const unsigned int ServerInfo::port() const
{
    return mPort;
}

//------------------------------------------------------------------------------
const size_t ServerInfo::maxConnections() const
{
    return mMaxConnections;
}

//------------------------------------------------------------------------------
const size_t ServerInfo::maxBacklog() const
{
    return mMaxBacklog;
}

//------------------------------------------------------------------------------
const size_t ServerInfo::bufferSize() const
{
    return mBufferSize;
}

}
}