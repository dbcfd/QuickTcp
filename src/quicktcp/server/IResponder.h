#pragma once

#include "quicktcp/server/Platform.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class SERVER_API IResponder {
public:       
    virtual ~IResponder();

    virtual bool authenticateConnection() = 0;
    virtual std::shared_ptr<utilities::ByteStream> respond(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void handleErrorAccepting(const std::string& message) = 0;
    virtual void handleErrorSendingResponse(const std::string& message) = 0;
    virtual void handleErrorIncompleteSend() = 0;
    virtual void handleConnectionClosed() = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}