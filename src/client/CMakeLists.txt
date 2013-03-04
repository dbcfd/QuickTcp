set (TARGET Client)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

SET (DEPENDENCIES Utilities)

add_library (${TARGET} ${LIBRARY_BUILD_TYPE} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/include FILES_MATCHING PATTERN "*.h" )
install (TARGETS ${TARGET} 
	RUNTIME DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/bin
	LIBRARY DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib
	ARCHIVE DESTINATION ${BASE_DIRECTORY}/install/${TARGET}/${version}/lib)

SetVSTargetProperties(${TARGET})

if(BUILD_TESTS)
	set(HEADERS)
	set(SOURCES)
	set(DEPENDENCIES)
	set(TARGET)
	add_subdirectory(test)
endif()