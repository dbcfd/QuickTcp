#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/ICompleter.h"

#include <atomic>
#include <memory>

namespace quicktcp {

namespace server {
class IResponder;
}

namespace os {
namespace windows {
namespace server {

class IEventHandler;
class ReceiveCompleter;

//------------------------------------------------------------------------------
class ConnectCompleter : public ICompleter {
public:
    ConnectCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler);
    virtual ~ConnectCompleter();

    virtual void handleIOCompletion(Overlap& holder, const size_t nbBytes) final;
    inline virtual bool readyForDeletion() const final;

    void reset(Overlap* holder);
    void handleConnection(Overlap& holder);
    void waitForDisconnect(Overlap& holder);

private:
    std::shared_ptr<ReceiveCompleter> mReceiver;
    bool mPendingDisconnect;
    bool mReadyForDeletion;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool ConnectCompleter::readyForDeletion() const
{
    return mReadyForDeletion;
}

}
}
}
}
