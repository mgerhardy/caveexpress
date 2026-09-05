# Wrapper around the official Android NDK CMake toolchain (same as libsdl-org/SDL CI).
#
# Required:
#   ANDROID_NDK_HOME or ANDROID_NDK — NDK root
#
# Optional:
#   ANDROID_ABI      (default: arm64-v8a)
#   ANDROID_PLATFORM (default: android-21)

if (DEFINED ENV{ANDROID_NDK_HOME} AND NOT "$ENV{ANDROID_NDK_HOME}" STREQUAL "")
	set(ANDROID_NDK "$ENV{ANDROID_NDK_HOME}")
elseif (DEFINED ENV{ANDROID_NDK} AND NOT "$ENV{ANDROID_NDK}" STREQUAL "")
	set(ANDROID_NDK "$ENV{ANDROID_NDK}")
elseif (NOT ANDROID_NDK)
	message(FATAL_ERROR "ANDROID_NDK_HOME (or ANDROID_NDK) is not set")
endif()

if (NOT ANDROID_PLATFORM)
	set(ANDROID_PLATFORM android-21)
endif()
if (NOT ANDROID_ABI)
	set(ANDROID_ABI arm64-v8a)
endif()
if (NOT ANDROID_STL)
	set(ANDROID_STL c++_static)
endif()
# Game code uses dynamic_cast; the NDK defaults to -fno-rtti.
if (NOT ANDROID_CPP_FEATURES)
	set(ANDROID_CPP_FEATURES rtti)
endif()

set(_CP_NDK_TOOLCHAIN "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
if (NOT EXISTS "${_CP_NDK_TOOLCHAIN}")
	message(FATAL_ERROR "NDK CMake toolchain not found: ${_CP_NDK_TOOLCHAIN}")
endif()
include("${_CP_NDK_TOOLCHAIN}")
