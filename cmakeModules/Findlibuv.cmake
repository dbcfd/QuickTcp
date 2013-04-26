# - Try to find libuv
#
#  This module defines the following variables
#
#  libuv_FOUND - Was library found
#  libuv_INCLUDE_DIRS - the include directories
#  libuv_LIBRARIES - Required libraries
#
#  This module accepts the following variables
#
#  libuv_ROOT - Set to location of libuv if not in PATH or current directory ThirdParty
#  THIRDPARTY_LIB_DIR - Location of third party directory where library might be
#
include(LibraryFinder)

if(libuv_VERSION)
	if(NOT libuv_VERSION STREQUAL libuv_FIND_VERSION)
		message(FATAL_ERROR "--libuv: Already found libuv with version ${libuv_VERSION} but now looking for ${libuv_FIND_VERSION}")
	endif()
endif()

if(NOT libuv_FOUND)
    set(FindProject "libuv")
    # Find the libraries

    _FIND_LIBRARY_FOR(${FindProject} LIBUV_LIBRARY libuv)
	_FIND_LIBRARY_FOR(${FindProject} LIBUV_LIBRARY_DEBUG libuv)

    # Find the headers
    _FIND_HEADER_FOR(${FindProject} "libuv/uv.h" LIBUV_INCLUDE_DIR)

    # handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
    # all listed variables are TRUE
    include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(${FindProject} DEFAULT_MSG
        LIBUV_LIBRARY LIBUV_INCLUDE_DIR)
    _APPEND_LIBRARIES(libuv_LIBRARIES LIBUV_LIBRARY)
    set(libuv_LIBRARIES ${libuv_LIBRARIES} CACHE INTERNAL "Listing of libuv libraries")
	set(libuv_INCLUDE_DIRS ${${FindProject}_INCLUDE_DIR} CACHE INTERNAL "libuv include directory")
	set(libuv_VERSION ${libuv_FIND_VERSION} CACHE INTERNAL "Version of libuv found")
	set(libuv_FOUND ${libuv_FOUND} CACHE INTERNAL "Whether or not libuv was found")
endif()

if(libuv_FOUND)
	include_directories(${libuv_INCLUDE_DIRS})
endif()