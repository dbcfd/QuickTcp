#pragma once

#include "quicktcp/client/Platform.h"
#include "async/Async.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

class CLIENT_API IProcessor {
public:
    virtual ~IProcessor();
    /**
     * Process a response from the server. This is used to fulfill responses of client request actions.
     */
    virtual async_cpp::async::AsyncResult processResponse(std::shared_ptr<utilities::ByteStream> stream) = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}