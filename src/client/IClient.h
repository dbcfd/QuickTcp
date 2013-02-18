#pragma once

#include "client/Platform.h"
#include "client/ServerInfo.h"

#include <future>
#include <memory>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace workers {
class Manager;
}

namespace client {
class CLIENT_API IClient {
public:
    class CLIENT_API SendResult
    {
    public:
        SendResult();
        SendResult(const std::string& err);

        inline void check() const;

    private:
        std::string mError;
    };

    IClient(std::shared_ptr<workers::Manager> manager, 
        const ServerInfo& info, 
        std::function<void(std::shared_ptr<utilities::ByteStream>)> onReceive);

    std::future<SendResult> send(std::shared_ptr<utilities::ByteStream> stream);

    virtual void disconnect() = 0;

    inline std::shared_ptr<workers::Manager> manager() const;

protected:
    void receive(std::shared_ptr<utilities::ByteStream> stream) const;

    virtual void performSend(std::shared_ptr<std::promise<SendResult>> responsePromise, std::shared_ptr<utilities::ByteStream> stream) = 0;

private:
   std::shared_ptr<workers::Manager> mManager;
   std::function<void(std::shared_ptr<utilities::ByteStream>)> mOnReceive;
   ServerInfo mInfo;
};

//inline implementations
//------------------------------------------------------------------------------
void IClient::SendResult::check() const
{
    if(!mError.empty())
    {
        throw(std::runtime_error(mError));
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<workers::Manager> IClient::manager() const
{
    return mManager;
}

}
}