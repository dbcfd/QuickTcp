#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/ICompleter.h"

#include "async/AsyncResult.h"

#include <functional>

namespace quicktcp {

namespace server {
class IResponder;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct Overlap;

//------------------------------------------------------------------------------
class ResponseCompleter : public ICompleter {
public:
    ResponseCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler);
    virtual ~ResponseCompleter();

    virtual void handleIOCompletion(Overlap& holder, const size_t nbBytes);
    inline virtual bool readyForDeletion() const final;
    inline void setResult(async_cpp::async::AsyncResult&& result);

private:
    async_cpp::async::AsyncResult mResult;
    bool mReadyForDeletion;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool ResponseCompleter::readyForDeletion() const
{
    return mReadyForDeletion;
}

//------------------------------------------------------------------------------
void ResponseCompleter::setResult(async_cpp::async::AsyncResult&& result)
{
    mResult = result;
}

}
}
}
}
