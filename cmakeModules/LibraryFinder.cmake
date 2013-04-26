#various macros for finding libraries

macro(_FIND_LIBRARY_FOR _proj _var)
	find_library(${_var}
		NAMES 
			${ARGN}
		HINTS
			${${_proj}_ROOT}/${${_proj}_FIND_VERSION}/lib
			${THIRDPARTY_LIB_DIR}/${_proj}/${${_proj}_VERSION}/lib
		PATH_SUFFIXES lib
	)
	if(NOT ${_var})
		message("--${_proj} : Failed to find ${ARGN} in ${${_proj}_ROOT}/${${_proj}_FIND_VERSION}/lib or ${THIRDPARTY_LIB_DIR}/${_proj}/${${_proj}_VERSION}/lib")
	endif()
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
			${${_proj}_ROOT}/${${_proj}_FIND_VERSION}/include
			${THIRDPARTY_LIB_DIR}/${_proj}/${${_proj}_VERSION}/include
	)
	set(${_include_dir} ${proj_include_dir})
endmacro()