#pragma once

#include "quicktcp/client/Platform.h"
#include "async_cpp/async/Async.h"

#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

template<class T = utilities::ByteStream>
class CLIENT_API IProcessor {
public:
    virtual ~IProcessor() {}
    /**
     * Process a response from the server. This is used to fulfill responses of client request actions.
     */
    virtual async_cpp::async::AsyncResult<T> processResponse(std::shared_ptr<utilities::ByteStream> stream) = 0;

    virtual void handleDisconnect() = 0;
    virtual void handleErrorResolveAddress(const std::string& message) = 0;
    virtual void handleErrorConnect(const std::string& message) = 0;
    virtual void handleErrorReceive(const std::string& message) = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}