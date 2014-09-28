cmake_minimum_required(VERSION 2.6)

option(RELEASE "Build a release build" OFF)

#configure_file("${PROJECT_SOURCE_DIR}/src/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/libs")

if(CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE Debug)
endif()

if(UNIX)
include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
endif(UNIX)

if(WIN32)
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/general")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/src")
endif(WIN32)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/cavepacker)
