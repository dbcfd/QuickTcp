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
#  THIRDPARTY_DIR - Location of third party directory where library might be
#
include(LibraryFinder)

if(NOT ASYNC.CPP_found)
    set(FindProject "async.cpp")
    # Find the libraries

    _FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY Async${LIB_CONVENTION})
    _FIND_LIBRARY_FOR(${FindProject} ASYNC_LIBRARY_DEBUG Async${DEBUG_LIB_CONVENTION})
    _FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY Workers${LIB_CONVENTION})
    _FIND_LIBRARY_FOR(${FindProject} WORKERS_LIBRARY_DEBUG Workers${DEBUG_LIB_CONVENTION})

    # Find the headers
    _FIND_HEADER_FOR(${FindProject} "async/Async.h" ${FindProject}_INCLUDE_DIR)

    # handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
    # all listed variables are TRUE
    include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(${FindProject} DEFAULT_MSG
        ASYNC_LIBRARY WORKERS_LIBRARY ${FindProject}_INCLUDE_DIR)
    _APPEND_LIBRARIES(ASYNC.CPP_LIBRARIES ASYNC_LIBRARY)
	_APPEND_LIBRARIES(ASYNC.CPP_LIBRARIES WORKERS_LIBRARY)
    set(ASYNC.CPP_LIBRARIES ${ASYNC.CPP_LIBRARIES} CACHE INTERNAL "Listing of Async.cpp libraries")
	set(ASYNC.CPP_INCLUDE_DIRS ${${FindProject}_INCLUDE_DIR} CACHE INTERNAL "Async.cpp include directory")
endif()

if(ASYNC.CPP_FOUND)
	include_directories(${ASYNC.CPP_INCLUDE_DIRS})
endif()