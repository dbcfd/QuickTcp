#
#  DCL_FOUND - Was library found
#  DCL_INCLUDE_DIRS - the include directories
#  DCL_LIBRARIES - Required libraries
#
#  This module accepts the following variables
#
#  DCL_DIR - Set to location of DCL if not in PATH or current directory ThirdParty
#  DCL_VERSION - Version of DCL to look for
#  THIRDPARTY_DIR - Location of third party directory to perform checkouts int
#

macro(_FIND_LIBRARY_FOR _proj _var)
	find_library(${_var}
		NAMES 
			${ARGN}
		HINTS
			${${_proj}_DIR}/lib
			${THIRDPARTY_DIR}/${_proj}/${${_proj}_VERSION}/lib
		PATH_SUFFIXES lib
	)
	mark_as_advanced(${_var})
endmacro()

macro(_APPEND_LIBRARIES _list _release)
	set(_debug ${_release}_DEBUG)
	if(${_debug})
		set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
	else()
		set(${_list} ${${_list}} ${${_release}})
	endif()
endmacro()

macro(_FIND_HEADER_FOR _proj _file _include_dir)
	find_path(proj_include_dir NAMES ${_file}
		HINTS
			${${_proj}_DIR}/include
			${THIRDPARTY_DIR}/${_proj}/${${_proj}_VERSION}/include
	)
	set(${_include_dir} ${proj_include_dir})
endmacro()