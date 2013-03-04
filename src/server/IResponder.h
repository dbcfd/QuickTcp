#pragma once

#include "server/Platform.h"

#include <future>
#include <memory>

namespace async_cpp {
namespace async {
class AsyncResult;
}
}

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class SERVER_API IResponder {
public:
    virtual std::future<async_cpp::async::AsyncResult> respond(std::shared_ptr<utilities::ByteStream> stream) = 0;
    virtual void connectionClosed() = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}