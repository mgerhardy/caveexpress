cmake_minimum_required(VERSION 2.6)

option(RELEASE "Build a release build" OFF)

set(CMAKE_CXX_FLAGS "-std=c++11") 

#configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/libs")

if(CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories("${PROJECT_SOURCE_DIR}/src/libs/lua")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/src")
if(WIN32)
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/general")
else()
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/linux")
endif(WIN32)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/cavepacker)
