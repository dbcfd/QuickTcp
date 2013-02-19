#pragma once

#include "server/Platform.h"
#include "server/ServerInfo.h"

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>

namespace quicktcp {

namespace utilities {
class ByteStream;
}

namespace workers {
class Manager;
}

namespace server {

class IServerConnection;

/**
 * Server interface. Platform specific implementations will implement this interface, allowing
 * common access to the platform servers
 */
class SERVER_API IServer {
public:
    class SERVER_API SendComplete
    {
    public:
        SendComplete();
        SendComplete(const std::string& error);

        inline const std::string& error() const;

    private:
        std::string mError;
    };

    IServer(const ServerInfo& info, std::shared_ptr<workers::Manager> manager, std::function<std::unique_ptr<IServerConnection>()> createConnectionFunc);

    void shutdown();
    std::future<SendComplete> send(std::shared_ptr<utilities::ByteStream> stream);

    inline const bool isRunning() const;
    inline std::shared_ptr<workers::Manager> manager() const;

protected:
    std::unique_ptr<IServerConnection> addConnection();

private:
    virtual void performShutdown() = 0;
    virtual void performSend(std::unique_ptr<std::promise<SendComplete>> promise, std::shared_ptr<utilities::ByteStream> stream) = 0;

    std::shared_ptr<workers::Manager> mManager;
    std::function<std::unique_ptr<IServerConnection>()> mCreateConnectionFunc;
    ServerInfo mInfo;

    std::atomic<bool> mIsRunning;
};

//inline implementations
//------------------------------------------------------------------------------
const std::string& IServer::SendComplete::error() const
{
    return mError;
}

//------------------------------------------------------------------------------
const bool IServer::isRunning() const
{
    return mIsRunning;
}

//------------------------------------------------------------------------------
std::shared_ptr<workers::Manager> IServer::manager() const
{
    return mManager;
}

}
}