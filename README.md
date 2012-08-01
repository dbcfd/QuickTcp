# QuickTcp #

## Overview ##
Tcp Server implemented using C++11 features. QuickTcp consists of the following libraries:
* Utilities (Stand-alone)
* Workers (Stand-alone)
* Server (Depends on Workers and Utilities)
* Client (Depends on Workers and Utilities)

### Utilities ###
Utilities objects to be used by Server and Client. Includes the following:
* ByteStream : Wrapper around a byte stream from a tcp connection
* BinarySerializable : Interface to serialize an object to binary. Includes conversion to ByteStream
* RequestToResponse : Function definition for handling an incoming tcp request, and producing an outgoing tcp response

### Workers ###
Task based work system to simplify threading.
* Task : Interface for any work which needs to be accomplished in a threaded manner. Can also be run non-threaded
* Worker : Threaded object to handle performing tasks
* WorkerPool : Collection of workers. Provides easy setup of a number of workers

### Server ###
TCP Server, uses ServerInterface and a platform specific implementation of ServerInterface

### Client ###
TCP Client, uses ClientInterface and a platform specific implementation of ClientInterface

## Build Instructions ##
Obtain a C++11 compatible compiler (VS2011, Gcc) and CMake 2.8.4 or higher. Run Cmake (preferably from the build directory).

See http://www.cmake.org for further instructions on CMake.

## Testing Instructions ##
