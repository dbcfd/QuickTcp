# - Try to find async.cpp
#
#  This module defines the following variables
#
#  async.cpp_FOUND - Was library found
#  async.cpp_INCLUDE_DIRS - the include directories
#  async.cpp_LIBRARIES - Required libraries
#
#  This module accepts the following variables
#
#  async.cpp_ROOT - Set to location of async.cpp if not in PATH or current directory ThirdParty
#  THIRDPARTY_LIB_DIR - Location of third party directory where library might be
#
include(LibraryFinder)

if(async.cpp_VERSION)
	if(NOT async.cpp_VERSION STREQUAL async.cpp_FIND_VERSION)
		message(FATAL_ERROR "--async.cpp: Already found async.cpp with version ${async.cpp_VERSION} but now looking for ${async.cpp_FIND_VERSION}")
	endif()
endif()

if(NOT async.cpp_FOUND)
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
    _APPEND_LIBRARIES(async.cpp_LIBRARIES ASYNC_LIBRARY)
	_APPEND_LIBRARIES(async.cpp_LIBRARIES WORKERS_LIBRARY)
    set(async.cpp_LIBRARIES ${async.cpp_LIBRARIES} CACHE INTERNAL "Listing of Async.cpp libraries")
	set(async.cpp_INCLUDE_DIRS ${${FindProject}_INCLUDE_DIR} CACHE INTERNAL "Async.cpp include directory")
	set(async.cpp_VERSION ${async.cpp_FIND_VERSION} CACHE INTERNAL "Version of async.cp found")
	set(async.cpp_FOUND ${ASYNC.CPP_FOUND} CACHE INTERNAL "Whether or not async.cpp was found")
endif()

if(async.cpp_FOUND)
	set(async.cpp_ROOT ${async.cpp_ROOT})
	include_directories(${async.cpp_INCLUDE_DIRS})
endif()