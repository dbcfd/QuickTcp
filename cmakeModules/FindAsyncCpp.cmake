# - Try to find AsyncCpp
#
#  This module defines the following variables
#
#  AsyncCpp_FOUND - Was library found
#  AsyncCpp_INCLUDE_DIRS - the include directories
#  AsyncCpp_LIBRARIES - Required libraries
#
#  This module accepts the following variables
#
#  AsyncCpp_ROOT - Set to location of AsyncCpp if not in PATH or current directory ThirdParty
#  THIRDPARTY_LIB_DIR - Location of third party directory where library might be
#
include(LibraryFinder)

if(AsyncCpp_VERSION)
	if(NOT AsyncCpp_VERSION STREQUAL AsyncCpp_FIND_VERSION)
		message(FATAL_ERROR "--AsyncCpp: Already found AsyncCpp with version ${AsyncCpp_VERSION} but now looking for ${AsyncCpp_FIND_VERSION}")
	endif()
endif()

if(NOT AsyncCpp_FOUND)
    # Find the libraries

    _FIND_LIBRARY_FOR(AsyncCpp ASYNC_LIBRARY Async${LIB_CONVENTION})
    _FIND_LIBRARY_FOR(AsyncCpp ASYNC_LIBRARY_DEBUG Async${DEBUG_LIB_CONVENTION})
    _FIND_LIBRARY_FOR(AsyncCpp TASKS_LIBRARY Tasks${LIB_CONVENTION})
    _FIND_LIBRARY_FOR(AsyncCpp TASKS_LIBRARY_DEBUG Tasks${DEBUG_LIB_CONVENTION})

    # Find the headers
    _FIND_HEADER_FOR(AsyncCpp "async_cpp/async/Async.h" ASYNC_INCLUDE_DIR)

    # handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
    # all listed variables are TRUE
    include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(AsyncCpp DEFAULT_MSG
        ASYNC_LIBRARY TASKS_LIBRARY ASYNC_INCLUDE_DIR)
    _APPEND_LIBRARIES(AsyncCpp_LIBRARIES ASYNC_LIBRARY)
	_APPEND_LIBRARIES(AsyncCpp_LIBRARIES TASKS_LIBRARY)
    set(AsyncCpp_LIBRARIES ${AsyncCpp_LIBRARIES} CACHE INTERNAL "Listing of AsyncCpp libraries")
	set(AsyncCpp_INCLUDE_DIRS ${ASYNC_INCLUDE_DIR} CACHE INTERNAL "AsyncCpp include directory")
	set(AsyncCpp_VERSION ${AsyncCpp_FIND_VERSION} CACHE INTERNAL "Version of async.cp found")
	set(AsyncCpp_FOUND ${AsyncCpp_FOUND} CACHE INTERNAL "Whether or not AsyncCpp was found")
endif()

if(AsyncCpp_FOUND)
	set(AsyncCpp_ROOT ${AsyncCpp_ROOT} CACHE INTERNAL "Async.cpp Root directory")
	include_directories(${AsyncCpp_INCLUDE_DIRS})
endif()