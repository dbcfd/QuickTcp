# - Try to find Winsock2
#
#  This module defines the following variables
#
#  Winsock2_FOUND - Was library found
#  Winsock2_INCLUDE_DIRS - the include directories
#  Winsock2_LIBRARIES - Required libraries
#  Winsock2_LINK_ONLY - Only use winsock2 library, not header
#
#  This module accepts the following variables
#
#  Winsock2_LIB_DIR - Set to location of Winsock2 if not in PATH or current directory ThirdParty
#  Winsock2_INC_DIR - Set to location of Winsock2 if not in PATH or current directory ThirdPartys
#  THIRDPARTY_LIB_DIR - Location of third party directory to perform checkouts int
#

macro(_FIND_Winsock2_LIBRARY _var)
	if(MSVC)
		set(${_var} ${ARGN})
		mark_as_advanced(${_var})
	endif()
endmacro()

macro(_Winsock2_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

macro(_FIND_Winsock2_HEADER _include_dir)
	find_path(winsock2_include_dir NAMES winsock2.h
		HINTS
			${Winsock2_INC_DIR}
			${THIRDPARTY_LIB_DIR}/Winsock2/${Winsock2_FIND_VERSION}/include
	)
	set(${_include_dir} ${winsock2_include_dir})
endmacro()

if(NOT WINSOCK2_FOUND)
    # Find the libraries

    if(CMAKE_SIZEOF_VOID_P EQUAL 8) 
        _FIND_Winsock2_LIBRARY(Winsock2_LIBRARY        ws2_32)
        _FIND_Winsock2_LIBRARY(Winsock2_LIBRARY_DEBUG  ws2_32)
    else()
        _FIND_Winsock2_LIBRARY(Winsock2_LIBRARY        ws2_32)
        _FIND_Winsock2_LIBRARY(Winsock2_LIBRARY_DEBUG  ws2_32)
    endif()

    # Find the headers
    _FIND_Winsock2_HEADER(Winsock2_INCLUDE_DIR)

    # handle the QUIETLY and REQUIRED arguments and set SomeMarkitLibrary_FOUND to TRUE if 
    # all listed variables are TRUE
    include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

    if(Winsock2_LINK_ONLY)
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Winsock2 DEFAULT_MSG
            Winsock2_LIBRARY)
    else()
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Winsock2 DEFAULT_MSG
            Winsock2_LIBRARY Winsock2_INCLUDE_DIR)
    endif()
    _Winsock2_APPEND_LIBRARIES(Winsock2_LIBRARIES Winsock2_LIBRARY)
    set(Winsock2_LIBRARIES ${Winsock2_LIBRARIES} CACHE INTERNAL "Winsock2 libraries")
    set(Winsock2_INCLUDE_DIRS ${Winsock2_INCLUDE_DIR} CACHE INTERNAL "Winsock2 include directories")
    set(Winsock2_FOUND ${WINSOCK2_FOUND} CACHE INTERNAL "Winsock2 found")
endif()

if(WINSOCK2_FOUND)
	if(NOT Winsock2_LINK_ONLY)
		include_directories(${Winsock2_INCLUDE_DIRS})
	endif()
endif()
