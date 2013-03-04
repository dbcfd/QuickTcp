# - Try to find async.cpp
#
#  This module defines the following variables
#
#  ASYNC.CPP_FOUND - Was library found
#  ASYNC.CPP_INCLUDE_DIRS - the include directories
#  ASYNC.CPP_LIBRARIES - Required libraries
#
#  This module accepts the following variables
#
#  async.cpp_DIR - Set to location of async.cpp if not in PATH or current directory ThirdParty
#  async.cpp_VERSION - Version of async.pp to look for
#  THIRDPARTY_DIR - Location of third party directory to perform checkouts int
#
include(LibraryFinder)

set(FindProject "async.cpp")
# Find the libraries

set(RUNTIME "MD")
if(USE_STATIC_RUNTIME)
	set(RUNTIME "MT")
endif()
set(STATIC_SUFFIX "")
if(NOT BUILD_SHARED_LIBS)
	set(STATIC_SUFFIX "-s")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
	_FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY Async-${RUNTIME}-x64${STATIC_SUFFIX})
	_FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY_DEBUG Async-${RUNTIME}-x64${STATIC_SUFFIX}-d)
	_FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY Workers-${RUNTIME}-x64${STATIC_SUFFIX})
	_FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY_DEBUG Workers-${RUNTIME}-x64${STATIC_SUFFIX}-d)
else()
	_FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY Async-${RUNTIME}-x86${STATIC_SUFFIX})
	_FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY_DEBUG Async-${RUNTIME}-x86${STATIC_SUFFIX}-d)
	_FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY Workers-${RUNTIME}-x86${STATIC_SUFFIX})
	_FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY_DEBUG Workers-${RUNTIME}-x86${STATIC_SUFFIX}-d)
endif()

# Find the headers
_FIND_HEADER_FOR(${FindProject} "async/Async.h" ${FindProject}_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(${FindProject} DEFAULT_MSG
    ASYNC_LIBRARY WORKERS_LIBRARY ${FindProject}_INCLUDE_DIR)
	
if(ASYNC.CPP_FOUND)
	_APPEND_LIBRARIES(ASYNC.CPP_LIBRARIES ASYNC_LIBRARY)
	_APPEND_LIBRARIES(ASYNC.CPP_LIBRARIES WORKERS_LIBRARY)
	set(ASYNC.CPP_INCLUDE_DIRS ${${FindProject}_INCLUDE_DIR})
	include_directories(${ASYNC.CPP_INCLUDE_DIRS})
endif()