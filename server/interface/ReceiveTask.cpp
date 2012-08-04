#include "server/interface/ReceiveTask.h"
#include "server/interface/IServerConnection.h"

namespace quicktcp {
namespace server {
namespace iface {

ReceiveTask::ReceiveTask(IServerConnection* connection, utilities::RequestToResponse reqToResp) : mConnection(connection), mReqToResp(reqToResp)
{

}

void ReceiveTask::performTask()
{
    utilities::ByteStream data = mConnection->blockingReceive();
    utilities::ByteStream resp = mReqToResp(data);
    if(resp.getSize() > 0)
    {
        mConnection->send(resp);
    }
}

}
}
}