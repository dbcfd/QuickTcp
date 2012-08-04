#pragma once

#include "server/interface/Platform.h"

#include "utilities/ByteStream.h"

#include "workers/Task.h"

namespace quicktcp {
namespace server {
namespace iface {

class IServerConnection;

class SendTask : public workers::Task
{
public:
    SendTask(IServerConnection* connection, const utilities::ByteStream& data);

    virtual void performTask();

    std::future<bool> getResult();
private:
    std::promise<bool> mSendResult;
    IServerConnection* mConnection;
    utilities::ByteStream mData;
};

}
}
}