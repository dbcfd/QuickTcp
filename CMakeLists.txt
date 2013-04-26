cmake_minimum_required(VERSION 2.8.4)

#includes
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmakeModules)
include(SetVSTargetProperties)
include(SetBuildProperties)

#project variables
project(QuickTcp CXX C)

set(PROJECT_VERSION 0.5.0)

option(BUILD_TESTS "Build tests" ON)
option(USE_STATIC_RUNTIME "Use the static runtime (/MT)" OFF)
option(BUILD_SHARED_LIBS "Build component libraries as shared libraries" ON)
#expose gtest option, allows static libs to use shared runtime
option(
  gtest_force_shared_crt
  "Use shared (DLL) run-time lib even when Google Test is built as static lib."
  OFF)
   
if(NOT USE_STATIC_RUNTIME AND NOT BUILD_SHARED_LIBS)
	set(gtest_force_shared_crt ON)
endif()

SetBuildProperties(${CMAKE_PROJECT_NAME})

set(outputBase "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${outputBase}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${outputBase}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${outputBase}/lib)

#paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRDPARTY_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

#third party libraries
set(gtest_VERSION 1.6.0)
add_subdirectory(thirdparty)

if(BUILD_TESTS)
	enable_testing()
endif()
#project source
find_package(libuv 0.9.1 REQUIRED)
find_package(async.cpp 0.7.0 REQUIRED)
add_subdirectory(src)