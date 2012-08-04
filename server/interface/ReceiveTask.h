#pragma once

#include "server/interface/Platform.h"

#include "utilities/ByteStream.h"
#include "utilities/RequestToResponse.h"

#include "workers/Task.h"

namespace quicktcp {
namespace server {
namespace iface {

class IServerConnection;

class ReceiveTask : public workers::Task
{
public:
    ReceiveTask(IServerConnection* connection, utilities::RequestToResponse reqToResp);

    virtual void performTask();

private:
    IServerConnection* mConnection;
    utilities::RequestToResponse mReqToResp;
};

}
}
}