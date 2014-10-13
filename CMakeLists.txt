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
include_directories("${PROJECT_SOURCE_DIR}/src/libs/tinyxml2")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/sqlite")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/gtest/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/gtest")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/libogg-1.3.1/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/libpng-1.6.2")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/yajl")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_mixer")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_net")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL_image")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/include")
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/src")

#-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE -DSQLITE_HAVE_ISNAN
#-lpthread
#-DLOAD_PNG -DPNG_NO_CONFIG_H -DHAVE_SDL_IMAGE_H

if(UNIX)
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/linux")
else()
include_directories("${PROJECT_SOURCE_DIR}/src/libs/SDL/general")
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/src/libs)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/cavepacker)
