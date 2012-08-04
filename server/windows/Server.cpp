#ifdef WINDOWS
#include "server/windows/Server.h"

#include "server/windows/ServerConnection.h"

namespace quicktcp {
namespace server {
namespace windows {


Server::Server(workers::WorkerPool* pool, const unsigned int port, const unsigned int backlog, const unsigned int maxUnusedConnections, iface::IServer::ConnectionAdded caFunc) :
    IServer(pool), mServerSocket(nullptr), mIncomingSocket(nullptr), mIgnoreClose(false), mMaxUnusedConnections(maxUnusedConnections), mUnusedConnections(0), mConnectionAdded(caFunc)
{
    //startup winsock
    WSAData wsaData;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw(std::runtime_error("WSAStartup Error"));
    }

    createOverlap(this);

    createServerSocket(port, backlog);

    prepareForServerConnection();
}

Server::~Server()
{
    disconnect();
    WSARecvDisconnect(mServerSocket->getSocket(), 0);
    WSASendDisconnect(mServerSocket->getSocket(), 0);
    mServerSocket->closeSocket();
    delete mServerSocket;
    mServerSocket = 0;
    WSARecvDisconnect(mIncomingSocket->getSocket(), 0);
    WSASendDisconnect(mIncomingSocket->getSocket(), 0);
    mIncomingSocket->closeSocket();
    delete mIncomingSocket;
    mIncomingSocket = 0;
}

void Server::disconnect()
{
    if(setRunning(false))
    {
        //we were running
        WSASetEvent(getOverlap()->hEvent);
        std::future<bool> fut = mDoneWaitingForEvents.get_future();
        fut.wait();
    }
    mIgnoreClose = true;
    mHandles.clear();
    std::for_each(mConnections.begin(), mConnections.end(), [](std::pair<const WSAEVENT, std::shared_ptr<iface::IServerConnection>> iter) {
        iter.second->close();
    } );
    mConnections.clear();
}

void Server::connectionClosed(WSAEVENT handle)
{
    if(!mIgnoreClose)
    {
        std::lock_guard<std::mutex> lock(mConnectionsMutex);
        ++mUnusedConnections;
        mConnections.erase(handle);
    }
}

HANDLE Server::performWaitForEvents()
{
    DWORD wait_rc = WSA_WAIT_IO_COMPLETION;

    /**
     * Wait until i/o is completed on the handle triggered. A handle
     * can be triggered without i/o being complete.
     */
    while(WSA_WAIT_IO_COMPLETION == wait_rc && isRunning()) //can receive shutdown while still waiting on io completion
    {
        if(WSA_WAIT_FAILED == (wait_rc = WSAWaitForMultipleEvents(mHandles.size(), &(mHandles[0]), FALSE, WSA_INFINITE, TRUE)))
        {
            throw(std::runtime_error("WSAWaitForMultipleEvents Error"));
        }
    }

    if(!isRunning())
    {
        return mHandles[0]; //return a handle within range if shut down
    }

    HANDLE handle = mHandles[(wait_rc - WSA_WAIT_EVENT_0)];

    if(mUnusedConnections > mMaxUnusedConnections)
    {
        //rebuild our handle array
        std::lock_guard<std::mutex> lock(mConnectionsMutex);
        mHandles.clear();
        mHandles.reserve(mConnections.size());
        std::for_each(mConnections.begin(), mConnections.end(), [this](std::pair<WSAEVENT,std::shared_ptr<iface::IServerConnection>> iter) {
            this->mHandles.push_back(iter.first);
        } );
    }
    return handle;
}

void Server::waitForEvents()
{
    if(!mHandles.empty())
    {
        setRunning(true);
        while(isRunning())
        {
            HANDLE handleId = performWaitForEvents();

            if(isRunning()) //if running, handle returned is valid and should be examined
            {
                WSAResetEvent(handleId);

                if(getOverlap()->hEvent == handleId)
                {
                    //if the event triggered is the server event, add a new connection
                    addNewConnection();
                }
                else
                {
                    std::shared_ptr<iface::IServerConnection> connection = mConnections[handleId];
                    if(nullptr != connection)
                    {
                        connection->receive();
                    }
                }
            }
        }
        mDoneWaitingForEvents.set_value(true);
    }
}

void Server::addNewConnection() throw(std::runtime_error)
{
    static ServerConnection::ConnectionClosed ccFunc = std::bind(&Server::connectionClosed, this, std::placeholders::_1);
    BOOL bOptVal = TRUE;
    int bOptLen = sizeof(BOOL);
    //update the socket context based on our server
    setsockopt(mIncomingSocket->getSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &bOptVal,
            bOptLen);
    ServerConnection* connection = new ServerConnection(*mIncomingSocket, generateIdentifier(), getWorkerPool(), ccFunc);
    std::shared_ptr<iface::IServerConnection> connectionPtr(connection);
    HANDLE handle = connection->getOverlap()->hEvent;
    mHandles.push_back(handle);
    mConnections.insert(std::make_pair(handle, connectionPtr));
    delete mIncomingSocket;
    mIncomingSocket = nullptr;

    DWORD flags = 0;
    DWORD nbBytes = 0;

    /**
     * When a connection is formed, we will be receiving information from that client at
     * some point in time. We queue a receive that will call back to a method that
     * can handle the connection information.
     */
    connection->receive();

    mConnectionAdded(connectionPtr);

    //prepare for another client to connect
    prepareForServerConnection();
}

void Server::createServerSocket(const unsigned int port, const unsigned int backlog)
{
    //WSA startup successful, create address info
    struct addrinfo* result = nullptr, *ptr = nullptr, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream sstr;
    sstr << port;

    int iResult = getaddrinfo("127.0.0.1", sstr.str().c_str(), &hints, &result);

    if(iResult != 0)
    {
        WSACleanup();
        throw(std::runtime_error("getaddreinfo Error"));
    }

    //getaddrinfo successful, create socket
    mServerSocket = new Socket();

    //socket creation successful, bind to socket
    iResult = bind(mServerSocket->getSocket(), result->ai_addr, (int) result->ai_addrlen);
    freeaddrinfo(result);

    if(SOCKET_ERROR == iResult)
    {
        
        throw(std::runtime_error("bind error"));
    }

    if(SOCKET_ERROR == listen(mServerSocket->getSocket(), backlog))
    {
        std::stringstream sstr;
        sstr << "listen Error " << WSAGetLastError();
        mServerSocket->closeSocket();
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }
}

void Server::prepareForServerConnection() throw(std::runtime_error)
{
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0, connectionBytesRead = 0;

    //create a socket to accept the connection on
    mIncomingSocket = new Socket();

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR
            == WSAIoctl(mServerSocket->getSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &acceptex_guid, sizeof(acceptex_guid), &pfnAcceptEx, sizeof(pfnAcceptEx),
                    &bytes, nullptr, nullptr))
    {
        throw(std::runtime_error("Failed to obtain AcceptEx() pointer"));
    }

    /**
     * Setup accept ex to accept a connection to the server asynchronously when
     * it occurs. We read nothing from the socket, since we want to accept immediately,
     * then acknowledge to the client that a connection has occurred.
     */
    if(!pfnAcceptEx(mServerSocket->getSocket(), mIncomingSocket->getSocket(), mAcceptExBuffer,
            0, //read nothing from the socket
            sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &connectionBytesRead,
            getOverlap()))
    {

        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            throw(std::runtime_error("AcceptEx Error()"));
        }
    }
}

}
}
}

#endif
