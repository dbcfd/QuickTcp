# QuickTcp #
C++11 Tcp Server using boost::asio for networking and Async.cpp for multi-threaded and asynchronous operations. Utilizes simple interfaces to allow for quick setup of tcp clients and servers that are fully asynchronous. Management of multiple simultaneous requests is built in.

## Overview ##
Consists of three libraries
* Utilities (Stand-alone)
* Server (Depends on Utilities)
* Client (Depends on Utilities)

### Utilities ###
Utilities objects to be used by Server and Client. Includes the following:
* ByteStream : Wrapper around a byte stream from a tcp connection
* BinarySerializer : Object responsible for serializing PODs to a byte array, which can then be converted to a ByteStream for use by networking operations
* ISerializable : Interface to be used by objects which needs data serialization using BinarySerializer

### Server ###
Implementation of asynchronous server. Requires the implementation of the following interfaces
 * IResponder : Authenticates connections, creates a response bytestream from a client request bytestream

### Client ###
Implementation of asynchronous client. Requires the implementation of the following interfaces
 * IAuthenticator : Perform more complex authentication with a server using boost::asio operations
 * IProcessor : Convert a response bytestream from a server into a desired response, based on the client

## Build Instructions ##
Obtain a C++11 compatible compiler (VS2011, Gcc) and CMake 2.8.4 or higher. Run Cmake (preferably from the build directory).

See http://www.cmake.org for further instructions on CMake.
