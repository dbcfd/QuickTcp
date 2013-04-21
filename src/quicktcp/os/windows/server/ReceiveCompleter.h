#pragma once

#include "quicktcp/os/windows/server/Platform.h"
#include "quicktcp/os/windows/server/ICompleter.h"

#include <atomic>
#include <functional>
#include <memory>

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
struct Overlap;

//------------------------------------------------------------------------------
class ReceiveCompleter : public ICompleter {
public:
    ReceiveCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler, 
        std::function<void(void)> onDisconnect);
    virtual ~ReceiveCompleter();

    virtual void handleIOCompletion(Overlap& holder, const size_t nbBytes) final;
    inline virtual bool readyForDeletion() const final;
    
    void prepareToReceive(Overlap& holder);
    void prepareToRespond(Overlap& holder);
    void disconnect();

private:
    std::function<void(void)> mOnDisconnect;
    bool mReadyForDeletion;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool ReceiveCompleter::readyForDeletion() const
{
    return mReadyForDeletion;
}

}
}
}
}
