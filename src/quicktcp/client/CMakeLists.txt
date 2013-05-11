set (TARGET Client)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${AsyncCpp_INCLUDE_DIRS})
SET (DEPENDENCIES ${AsyncCpp_LIBRARIES} Utilities ${Boost_LIBRARIES})
if(UNIX)
	set(DEPENDENCIES ${DEPENDENCIES})
else()
	set(DEPENDENCIES ${DEPENDENCIES} ws2_32 Iphlpapi psapi)
endif()

if(BUILD_SHARED_LIBS)
	add_definitions("-DBOOST_ALL_DYN_LINK")
endif()

add_library (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_VERSION}
	DEBUG_OUTPUT_NAME ${TARGET}${DEBUG_LIB_CONVENTION}
	OUTPUT_NAME ${TARGET}${LIB_CONVENTION})

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/include FILES_MATCHING PATTERN "*.h" )
install (TARGETS ${TARGET} 
	RUNTIME DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/bin
	LIBRARY DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib
	ARCHIVE DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib)
    
include(DllInstaller)
_COPY_DLLS(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${AsyncCpp_LIBRARIES} ${Boost_LIBRARIES})

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()