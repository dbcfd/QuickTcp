set (TARGET WindowsClient)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

set(async.cpp_VERSION 0.6.0)
find_package(async.cpp REQUIRED)
find_package(Winsock2)
SET (DEPENDENCIES ${WINSOCK2_LIBRARIES} ${ASYNC.CPP_LIBRARIES} Client Utilities)

add_library (${TARGET} ${LIBRARY_BUILD_TYPE} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

include(DllInstaller)
_COPY_DLLS(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${ASYNC.CPP_LIBRARIES})

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()