add_subdirectory(utilities)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(os)

set (TARGET QuickTcp)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

if(UNIX)
	SET (DEPENDENCIES PosixServer PosixClient)
else()
	SET (DEPENDENCIES WindowsServer WindowsClient)
endif()

include_directories(${async.cpp_INCLUDE_DIRS})
SET (DEPENDENCIES ${DEPENDENCIES} Utilities ${async.cpp_LIBRARIES})

add_library (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION}
	DEBUG_OUTPUT_NAME ${TARGET}${DEBUG_LIB_CONVENTION}
	OUTPUT_NAME ${TARGET}${LIB_CONVENTION})

if(NOT MSVC)
	install (TARGETS ${TARGET} 
		RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
		LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
		ARCHIVE DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
else()
	foreach(config ${CMAKE_CONFIGURATION_TYPES})
		install (TARGETS ${TARGET} 
			RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${config} 
			LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${config} 
			ARCHIVE DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${config})
	endforeach()
endif()

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()