cmake_minimum_required(VERSION 3.15)
project(caveproductions)

#-------------------------------------------------------------------------------
# options
#-------------------------------------------------------------------------------

option(NETWORKING "Enable networking" ON)
option(UNITTESTS "Builds with unittests" ON)
option(TOOLS "Builds with tools" ON)
option(USE_CCACHE "Use ccache" ON)
# The build system will search for system libs, if it can't find them (e.g. via pkg-config)
# it will take those that are bundled under src/libs. If you don't want that for some reason,
# you can set this to ON and the libs dir will not be taken into account at all.
option(FORCE_USE_SYSTEM_LIBS "Use system libs" OFF)
option(CAVEEXPRESS "Build caveexpress" ON)
option(CAVEPACKER "Build cavepacker" ON)
option(SANITIZER "Build with sanitizer support if available" OFF)

#-------------------------------------------------------------------------------
# end options
#-------------------------------------------------------------------------------

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Compile Type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(RELEASE False)
else()
	set(RELEASE True)
endif()

if (BUILD_SHARED_LIBS)
	message(STATUS "Forcing building the internal libraries as static, even though BUILD_SHARED_LIBS=ON was requested.")
	set(BUILD_SHARED_LIBS OFF)
endif()

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Root dir")

message(STATUS "Place binaries in ${ROOT_DIR}")
# First for the generic no-config case (e.g. with mingw)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR})
# Second, for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${ROOT_DIR})
endforeach()

set(DEFAULT_LUAC_EXECUTABLE luac)
set(CP_CMAKE_DIR ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_program(GDB_EXECUTABLE gdb)
find_program(LLDB_EXECUTABLE lldb)
if (GDB_EXECUTABLE)
	set(DEBUGGER ${GDB_EXECUTABLE} CACHE STRING "Which debugger should be used")
elseif (LLDB_EXECUTABLE)
	set(DEBUGGER ${LLDB_EXECUTABLE} CACHE STRING "Which debugger should be used")
else()
	set(DEBUGGER "unknown" CACHE STRING "Which debugger should be used")
	message(STATUS "No debugger (gdb or lldb) was found")
endif()
set_property(CACHE DEBUGGER PROPERTY STRINGS gdb lldb)

include(${CP_CMAKE_DIR}/macros.cmake)

cp_message("ROOT_DIR: ${ROOT_DIR}")
cp_message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
cp_message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")

#-------------------------------------------------------------------------------
# variables
#-------------------------------------------------------------------------------

if (WIN32)
	set(WINDOWS 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	set(LINUX 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	set(DARWIN 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	set(EMSCRIPTEN 1)
else()
	message(WARNING "Unknown host system: ${CMAKE_SYSTEM_NAME}. Default to linux")
	set(LINUX 1)
endif()

include(${CP_CMAKE_DIR}/pack.cmake)
include(CPack)

if (NOT CMAKE_TOOLCHAIN_FILE)
	if (WINDOWS)
		include(${CP_CMAKE_DIR}/toolchains/windows-toolchain.cmake)
	elseif (DARWIN)
		include(${CP_CMAKE_DIR}/toolchains/darwin-toolchain.cmake)
	else()
		include(${CP_CMAKE_DIR}/toolchains/linux-toolchain.cmake)
	endif()
else()
	if (MSVC)
		# 4456: declaration of 'q' hides previous local declaration
		# 4244: possible loss of data (e.g. float/double)
		# 4201: nonstandard extension used
		# 4245: signed/unsigned mismatch
		# 4100: unreferenced formal parameter
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4245 /wd4201 /wd4100 /wd4456 /wd4267")
	endif()
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (ANDROID)
	include(${CP_CMAKE_DIR}/android.cmake)
elseif (DARWIN)
	include(${CP_CMAKE_DIR}/darwin.cmake)
elseif (WINDOWS)
#	include(${CP_CMAKE_DIR}/windows.cmake)
elseif (EMSCRIPTEN)
	include(${CP_CMAKE_DIR}/emscripten.cmake)
endif()

if (PKGDATADIR)
	add_definitions(-DPKGDATADIR="${PKGDATADIR}")
	message(STATUS "You specified a package data dir - the game will append the game name to this directory.")
	message(STATUS "So you get e.g. ${PKGDATADIR}/caveexpress as final directory.")
endif()

if (NOT NETWORKING)
	add_definitions(-DNONETWORK=1)
endif()

if (NOT CMAKE_BUILD_TYPE)
	if (RELEASE)
		set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Compile Type" FORCE)
	else()
		set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Compile Type" FORCE)
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

find_host_program(CCACHE "ccache")
if (CCACHE)
	if (USE_CCACHE)
		message(STATUS "Using ccache")
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE})
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE})
	endif()
else()
	message(STATUS "ccache not found")
endif()

add_subdirectory(src/libs)

configure_file(${ROOT_DIR}/src/config.h.in ${CMAKE_BINARY_DIR}/cp-config.h)
include_directories(${CMAKE_BINARY_DIR})

include_directories(src)
include_directories(src/modules)
include_directories(BEFORE src/modules/physics)
if (UNITTESTS)
	include_directories(src/libs/gtest)
	include_directories(src/libs/gtest/include)
endif()

# TODO move into lib cmakefiles
if (LUA_FOUND)
	add_definitions(-DHAVE_LUA_H)
endif()
if (UNITTESTS)
	enable_testing()
endif()
add_subdirectory(src/modules)
add_subdirectory(src/tools)
if (CAVEPACKER)
	add_subdirectory(src/cavepacker)
endif()
if (CAVEEXPRESS)
	add_subdirectory(src/caveexpress)
endif()
if (UNITTESTS)
	add_subdirectory(src/tests)
endif()
