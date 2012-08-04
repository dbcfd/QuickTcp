#pragma once

#include "server/interface/Platform.h"

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {
namespace iface {

class SERVER_INTERFACE_API IServerConnection;

/**
 * Callback used to identify users that actions have occurred on a connection.
 */
class SERVER_INTERFACE_API ICallback
{
public:
    ICallback()
    {
    }
    virtual ~ICallback()
    {
    }

    virtual void connectionEstablished(IServerConnection* connection);
    virtual void connectionTerminated(IServerConnection* connection);

};

}
}
}
