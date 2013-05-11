#pragma once

#include "quicktcp/client/Platform.h"

#include <boost/asio.hpp>
#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace client {

class CLIENT_API IAuthenticator {
public:
    virtual ~IAuthenticator();
    /**
     * Perform authentication using a socket
     */
    virtual bool authenticate(std::shared_ptr<boost::asio::ip::tcp::socket> socket) = 0;
};

//inline implementations
//------------------------------------------------------------------------------

}
}