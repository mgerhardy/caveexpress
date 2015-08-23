set(FRAMEWORKS Foundation OpenGLES CoreGraphics AudioToolbox UIKit GLKit QuartzCore CoreMotion)

set(CMAKE_OSX_ARCHITECTURES "armv7")
set(CMAKE_OSX_SYSROOT "iphoneos")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")

include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(clang Clang)
CMAKE_FORCE_CXX_COMPILER(clang++ Clang)

set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "NO")
set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
set(CMAKE_XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI "NO")

set(CMAKE_C_FLAGS "-fstrict-aliasing -Wno-multichar -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-ignored-qualifiers -Wno-long-long -Wno-overloaded-virtual -Wno-unused-volatile-lvalue -Wno-deprecated-writable-strings")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -g")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -Wnon-virtual-dtor -fno-rtti -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set(CMAKE_EXE_LINKER_FLAGS "-ObjC -dead_strip -lpthread")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
