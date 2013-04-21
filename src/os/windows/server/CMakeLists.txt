set (TARGET WindowsServer)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

include_directories(${async.cpp_INCLUDE_DIRS})
find_package(Winsock2)
SET (DEPENDENCIES Server Utilities ${WINSOCK2_LIBRARIES} ${async.cpp_LIBRARIES})

add_library (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION}
	DEBUG_OUTPUT_NAME ${TARGET}${DEBUG_LIB_CONVENTION}
	OUTPUT_NAME ${TARGET}${LIB_CONVENTION})

include(DllInstaller)
_COPY_DLLS(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${async.cpp_LIBRARIES})

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()