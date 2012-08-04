#include "server/interface/SendTask.h"
#include "server/interface/IServerConnection.h"

namespace quicktcp {
namespace server {
namespace iface {

SendTask::SendTask(IServerConnection* connection, const utilities::ByteStream& data) : mData(data), mConnection(connection)
{

}

void SendTask::performTask() 
{
    mSendResult.set_value(mConnection->blockingSend(mData));
}

std::future<bool> SendTask::getResult() 
{
    return mSendResult.get_future();
}

}
}
}