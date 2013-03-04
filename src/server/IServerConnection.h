#pragma once

#include "server/Platform.h"
#include "server/IServer.h"

#include "async/AsyncResult.h"

#include <future>
#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace server {

class IServer;
class IResponder;

class SERVER_API IServerConnection {
public:       
    IServerConnection(std::shared_ptr<IResponder> responder);
    virtual ~IServerConnection();  

    virtual void disconnect() = 0;

    std::future<async_cpp::async::AsyncResult> getResponse(std::shared_ptr<utilities::ByteStream> stream);

protected:
   std::shared_ptr<IResponder> mResponder;
};

//inline implementations
//------------------------------------------------------------------------------

}
}