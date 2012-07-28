add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY)

set (TARGET TestClient)

file(GLOB SOURCES "*.cpp")

find_package(gtest REQUIRED)

SET (DEPENDENCIES ${DEPENDENCIES} Client gtest)

add_executable (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})