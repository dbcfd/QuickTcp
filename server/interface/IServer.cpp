#include "server/interface/IServer.h"

#include <iomanip>
#include <sstream>

namespace quicktcp {
namespace server {
namespace iface {

IServer::IServer(workers::WorkerPool* pool) : 
    mWorkerPool(pool), mLastTime(std::chrono::system_clock::now()), mIdentifiersDuringTimeframe(0)
{
   
}

std::string IServer::generateIdentifier()
{
    std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
    if(0 != std::chrono::duration_cast<std::chrono::seconds>(now - mLastTime).count())
    {
        mLastTime = now;
        mLastTimeT = std::chrono::system_clock::to_time_t(mLastTime);
        mIdentifiersDuringTimeframe = 0;
    }
    std::stringstream sstr;
    struct tm* tmptr = nullptr;
    if(0 == localtime_s(tmptr, &mLastTimeT))
    {
        sstr << std::put_time(tmptr, "%F %T") << mIdentifiersDuringTimeframe;
    }
    return sstr.str();
}

}
}
}