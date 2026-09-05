# Native Android settings. The NDK toolchain must already be loaded.
# SDK / Java / Gradle are only required when packaging an APK.

if (NOT CMAKE_ANDROID_NDK AND DEFINED ENV{ANDROID_NDK_HOME})
	set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})
endif()
if (NOT CMAKE_ANDROID_NDK AND DEFINED ENV{ANDROID_NDK})
	set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})
endif()
if (NOT CMAKE_ANDROID_NDK)
	message(FATAL_ERROR "Android NDK not found. Set ANDROID_NDK_HOME or use the NDK CMake toolchain.")
endif()

if (DEFINED ENV{ANDROID_SDK_ROOT} AND NOT "$ENV{ANDROID_SDK_ROOT}" STREQUAL "")
	set(ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})
elseif (DEFINED ENV{ANDROID_HOME} AND NOT "$ENV{ANDROID_HOME}" STREQUAL "")
	set(ANDROID_SDK_ROOT $ENV{ANDROID_HOME})
else()
	set(ANDROID_SDK_ROOT "")
endif()

if (NOT CMAKE_ANDROID_ARCH_ABI)
	if (ANDROID_ABI)
		set(CMAKE_ANDROID_ARCH_ABI ${ANDROID_ABI})
	else()
		set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
	endif()
endif()

set(ANDROID_API "android-21" CACHE STRING "Android native API level")
set(ANDROID_ROOT ${ROOT_DIR}/android-project)

option(HD_VERSION "Build the HD versions of the games" OFF)
set(TOOLS OFF)
set(USE_BUILTIN ON)

add_definitions(-DGL_GLEXT_PROTOTYPES)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-frtti>)
link_libraries(atomic)

message(STATUS "Android NDK root: ${CMAKE_ANDROID_NDK}")
message(STATUS "Android ABI: ${CMAKE_ANDROID_ARCH_ABI}")
if (ANDROID_SDK_ROOT)
	message(STATUS "Android SDK root: ${ANDROID_SDK_ROOT}")
else()
	message(STATUS "Android SDK root: (not set — native build only)")
endif()

find_host_program(ANDROID_ADB adb PATHS ${ANDROID_SDK_ROOT}/platform-tools)
find_host_program(ANDROID_NDK_STACK ndk-stack HINTS ${CMAKE_ANDROID_NDK} ${CMAKE_ANDROID_NDK}/prebuilt)

set(ANDROID_GRADLEW ${ANDROID_ROOT}/gradlew)
if (NOT EXISTS ${ANDROID_GRADLEW})
	set(ANDROID_GRADLEW "")
endif()
