cmake_minimum_required(VERSION 2.6)

include(CPack)

set(ROOT_DIR ${PROJECT_SOURCE_DIR} CACHE STRING "Root dir")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR})
SET(DEFAULT_LUAC_EXECUTABLE luac)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ROOT_DIR}/cmake)
SET(CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES OFF)

option(RELEASE "Build a release build" OFF)
option(UNITTESTS "Builds with unittests" ON)
option(TOOLS "Builds with tools" ON)

include(${ROOT_DIR}/cmake/macros.cmake)

#-------------------------------------------------------------------------------
# functions (incl. overrides)
#-------------------------------------------------------------------------------

function(add_executable name)
	if (ANDROID)
		add_library(${name} SHARED ${ARGN})
	else()
		_add_executable(${name} ${ARGN})
	endif()
endfunction()

#-------------------------------------------------------------------------------
# variables
#-------------------------------------------------------------------------------

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(WINDOWS 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(DARWIN 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
else()
	message(WARNING "Unknown host system - default to linux")
	set(LINUX 1)
endif()
find_program(TEXTUREPACKER_BIN NAMES TexturePacker)
if (TEXTUREPACKER_BIN-NOTFOUND)
	message("Could not find TexturePacker")
else()
	message("Found TexturePacker: ${TEXTUREPACKER_BIN}")
endif()

#-------------------------------------------------------------------------------
# options
#-------------------------------------------------------------------------------

option(NETWORKING "Enable networking" ON)

#-------------------------------------------------------------------------------
# end options
#-------------------------------------------------------------------------------

if (NOT CMAKE_TOOLCHAIN_FILE)
	if (WINDOWS)
		include(${ROOT_DIR}/cmake/toolchains/windows-toolchain.cmake)
	elseif (DARWIN)
		include(${ROOT_DIR}/cmake/toolchains/darwin-toolchain.cmake)
	else()
		include(${ROOT_DIR}/cmake/toolchains/linux-toolchain.cmake)
	endif()
endif()

if (ANDROID)
	include(${ROOT_DIR}/cmake/android.cmake)
elseif (IOS)
	include(${ROOT_DIR}/cmake/ios.cmake)
elseif (WINDOWS)
	include(${ROOT_DIR}/cmake/toolchains/windows-toolchain.cmake)
elseif (NACL)
	include(${ROOT_DIR}/cmake/nacl.cmake)
elseif (EMSCRIPTEN)
	include(${ROOT_DIR}/cmake/emscripten.cmake)
endif()

if (NOT NETWORKING)
add_definitions(-DNONETWORK=1)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_program(CCACHE "ccache")
if (CCACHE)
	message("Using ccache")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
else()
	message(WARNING "ccache not found")
endif()

include_directories("${ROOT_DIR}/src")
include_directories("${ROOT_DIR}/src/modules")
include_directories("${ROOT_DIR}/src/modules/game")
include_directories("${ROOT_DIR}/src/libs/gtest")
include_directories("${ROOT_DIR}/src/libs/gtest/include")

add_subdirectory(src/libs)

include_directories("${ROOT_DIR}/src/libs")
include_directories("${LUA_INCLUDE_DIR}")
include_directories("${BOX2D_INCLUDE_DIR}")
include_directories("${ROOT_DIR}/src/libs/tinyxml2")
include_directories("${SQLITE3_INCLUDE_DIRS}")
include_directories("${ROOT_DIR}/src/libs/yajl")
include_directories("${SDL2_NET_INCLUDE_DIR}")
include_directories("${SDL2_MIXER_INCLUDE_DIR}")
include_directories("${SDL2_IMAGE_INCLUDE_DIR}")
include_directories("${SDL2_INCLUDE_DIR}")

if (LUA_FOUND)
add_definitions(-DHAVE_LUA_H)
endif()

if (UNITTESTS)
	enable_testing()
endif()
add_subdirectory(src/modules)
add_subdirectory(src/cavepacker)
add_subdirectory(src/caveexpress)
if (UNITTESTS)
	add_subdirectory(src/tests)
endif()
