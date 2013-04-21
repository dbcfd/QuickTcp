#pragma once

#include "os/windows/server/Platform.h"
#include "os/windows/server/ICompleter.h"

namespace quicktcp {
namespace os {
namespace windows {
namespace server {

class IEventHandler;
class Socket;
struct Overlap;

//------------------------------------------------------------------------------
class SendCompleter : public ICompleter {
public:
    SendCompleter(std::shared_ptr<Socket> sckt, 
        std::shared_ptr<IEventHandler> evHandler,
        const size_t expectedSize);
    ~SendCompleter();

    virtual void handleIOCompletion(Overlap& holder, const size_t nbBytes) final;
    inline virtual bool readyForDeletion() const final;

    void completeSend(Overlap& holder);

private:
    size_t mExpectedSize;
    size_t mTotalBytes;
    bool mReadyForDeletion;
};

//Inline Implementations
//------------------------------------------------------------------------------
bool SendCompleter::readyForDeletion() const
{
    return mReadyForDeletion;
}

}
}
}
}
