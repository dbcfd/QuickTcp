MACRO(_FIND_DLL_FOR _lib _dll)
	get_filename_component(UTF_BASE_NAME ${_lib} NAME_WE)
	get_filename_component(UTF_PATH ${_lib} PATH)
	find_path(lib_dll_path NAMES ${UTF_BASE_NAME}.dll
		HINTS
			${UTF_PATH}
			${UTF_PATH}/../bin
	)
	set(${_dll} "${lib_dll_path}/${UTF_BASE_NAME}.dll")
endmacro()

MACRO(_INSTALL_LIBRARY _target _library _config _destination)
	_FIND_DLL_FOR(${_library} FOUND_DLL)
	if(FOUND_DLL)
		get_filename_component(UTF_BASE_NAME ${FOUND_DLL} NAME_WE)
		add_custom_command(TARGET ${_target}
			COMMAND ${CMAKE_COMMAND} -E copy
			${FOUND_DLL}
			${_destination}/${_config}/${UTF_BASE_NAME}.dll
		)
	endif()
ENDMACRO()

macro(_INSTALL_LIBRARIES _target _dir)
	set(lib_debug ${ARGN})
	set(lib_optimized ${ARGN})
	string(REGEX REPLACE "optimized;[^;]+;?" "" lib_debug "${lib_debug}")
	string(REGEX REPLACE "debug;" ";" lib_debug "${lib_debug}")
	string(REGEX REPLACE "debug;[^;]+;?" "" lib_optimized "${lib_optimized}")
	string(REGEX REPLACE "optimized;" ";" lib_optimized "${lib_optimized}")
	foreach(build_config ${CMAKE_CONFIGURATION_TYPES})
		set(libsToInstall ${lib_optimized})
		if(build_config STREQUAL "Debug")
			set(libsToInstall ${lib_debug})
		endif()
		foreach(lib ${libsToInstall})
			_INSTALL_LIBRARY(${_target} ${lib} ${build_config} ${_dir})
		endforeach()
	endforeach()
endmacro() 