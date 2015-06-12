add_subdirectory(Box2D)
add_subdirectory(lua)
add_subdirectory(SDL_mixer)
add_subdirectory(SDL_image)

FILE(GLOB libs_SRCS
	SDL_net/SDLnet.c
	SDL_net/SDLnetselect.c
	SDL_net/SDLnetTCP.c
	SDL_net/SDLnetUDP.c
	tinyxml2/tinyxml2.cpp
	yajl/yajl_alloc.c
	yajl/yajl_buf.c
	yajl/yajl.c
	yajl/yajl_encode.c
	yajl/yajl_gen.c
	yajl/yajl_lex.c
	yajl/yajl_parser.c
	yajl/yajl_tree.c
	zlib/adler32.c
	zlib/compress.c
	zlib/crc32.c
	zlib/deflate.c
	zlib/gzclose.c
	zlib/gzlib.c
	zlib/gzread.c
	zlib/gzwrite.c
	zlib/infback.c
	zlib/inffast.c
	zlib/inflate.c
	zlib/inftrees.c
	zlib/trees.c
	zlib/uncompr.c
	zlib/zutil.c
	SDL/src/video/*.c
	SDL/src/video/raspberry/*.c
	SDL/src/video/dummy/*.c
	SDL/src/video/directfb/*.c
	SDL/src/video/pandora/*.c
	SDL/src/video/x11/*.c
	SDL/src/video/wayland/*.c
	SDL/src/video/bwindow/*.c
	SDL/src/file/*.c
	SDL/src/render/*.c
	SDL/src/render/opengl/*.c
	SDL/src/render/opengles/*.c
	SDL/src/render/software/*.c
	SDL/src/render/opengles2/*.c
	SDL/src/haptic/*.c
	SDL/src/haptic/dummy/*.c
	SDL/src/loadso/*.c
	SDL/src/loadso/dummy/*.c
	SDL/src/loadso/dlopen/*.c
	SDL/src/dynapi/*.c
	SDL/src/main/*.c
	SDL/src/timer/*.c
	SDL/src/timer/dummy/*.c
	SDL/src/filesystem/*.c
	SDL/src/filesystem/unix/*.c
	SDL/src/filesystem/dummy/*.c
	SDL/src/events/*.c
	SDL/src/joystick/*.c
	SDL/src/joystick/bsd/*.c
	SDL/src/joystick/dummy/*.c
	SDL/src/input/*.c
	SDL/src/input/evdev/*.c
	SDL/src/core/*.c
	SDL/src/cpuinfo/*.c
	SDL/src/libm/*.c
	SDL/src/power/*.c
	SDL/src/power/uikit/*.c
	SDL/src/audio/*.c
	SDL/src/audio/bsd/*.c
	SDL/src/audio/esd/*.c
	SDL/src/audio/dummy/*.c
	SDL/src/audio/dsp/*.c
	SDL/src/audio/paudio/*.c
	SDL/src/audio/baudio/*.c
	SDL/src/audio/sndio/*.c
	SDL/src/audio/disk/*.c
	SDL/src/audio/sun/*.c
	SDL/src/audio/qsa/*.c
	SDL/src/audio/fusionsound/*.c
	SDL/src/audio/nas/*.c
	SDL/src/atomic/*.c
	SDL/src/thread/*.c
	SDL/src/stdlib/*.c
	SDL/src/*.c
	sqlite/sqlite3.c
	gtest/src/gtest-all.cc
	)

if(ANDROID)
FILE(GLOB libs_PLATFORM_SRCS
	SDL/src/filesystem/android/*.c
	SDL/src/video/android/*.c
	SDL/src/core/android/*.c
	SDL/src/timer/unix/*.c
	SDL/src/main/android/*.c
	SDL/src/thread/pthread/*.c
	SDL/src/audio/android/*.c
	SDL/src/joystick/android/*.c
	SDL/src/power/android/*.c)
elseif(EMSCRIPTEN)
FILE(GLOB libs_PLATFORM_SRCS
	SDL/src/audio/emscripten/*.c
	SDL/src/filesystem/emscripten/*.c
	SDL/src/joystick/emscripten/*.c
	SDL/src/power/emscripten/*.c
	SDL/src/video/emscripten/*.c
	SDL/src/thread/generic/*.c
	SDL/src/timer/unix/*.c)
elseif(NACL)
FILE(GLOB libs_PLATFORM_SRCS
	SDL/src/audio/nacl/*.c
	SDL/src/filesystem/nacl/*.c
	SDL/src/video/nacl/*.c
	SDL/src/timer/unix/*.c
	SDL/src/thread/pthread/*.c
	SDL/src/main/nacl/*.c)
elseif(APPLE)
FILE(GLOB libs_PLATFORM_SRCS
	SDL_image/IMG_UIImage.m
	SDL/src/video/uikit/*.m
	SDL/src/video/cocoa/*.m
	SDL/src/file/cocoa/*.m
	SDL/src/filesystem/cocoa/*.m
	SDL/src/joystick/iphoneos/*.m
	SDL/src/power/uikit/*.m
	SDL/src/audio/coreaudio/*.c
	SDL/src/filesystem/cocoa/*.c
	SDL/src/joystick/iphoneos/*.c
	SDL/src/joystick/darwin/*.c
	SDL/src/timer/unix/*.c
	SDL/src/video/uikit/*.c
	SDL/src/main/dummy/*.c
	SDL/src/video/cocoa/*.c
	SDL/src/file/cocoa/*.c
	SDL/src/haptic/darwin/*.c
	SDL/src/power/macosx/*.c
	SDL/src/thread/pthread/*.c)
elseif(WIN32)
FILE(GLOB libs_PLATFORM_SRCS
	SDL/src/joystick/windows/*.c
	SDL/src/filesystem/windows/*.c
	SDL/src/haptic/windows/*.c
	SDL/src/video/windows/*.c
	SDL/src/loadso/windows/*.c
	SDL/src/main/windows/*.c
	SDL/src/timer/windows/*.c
	SDL/src/core/windows/*.c
	SDL/src/power/windows/*.c
	SDL/src/audio/winmm/*.c
	SDL/src/audio/directsound/*.c
	SDL/src/render/direct3d/*.c
	SDL/src/thread/windows/*.c
	SDL/src/thread/generic/SDL_syscond.c)
elseif(LINUX)
include(FindPkgConfig)
pkg_search_module(DBUS REQUIRED dbus-1 dbus)
include_directories(${DBUS_INCLUDE_DIRS})
FILE(GLOB libs_PLATFORM_SRCS
	SDL/src/thread/pthread/*.c
	SDL/src/haptic/linux/*.c
	SDL/src/joystick/linux/*.c
	SDL/src/audio/alsa/*.c
	SDL/src/audio/arts/*.c
	SDL/src/audio/pulseaudio/*.c
	SDL/src/timer/unix/*.c
	SDL/src/main/dummy/*.c
	SDL/src/core/linux/*.c
	SDL/src/power/linux/*.c)
endif()

add_definitions(-DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE -DSQLITE_HAVE_ISNAN -DSQLITE_THREADSAFE=0)
add_library(libs ${libs_SRCS} ${libs_PLATFORM_SRCS})
set_target_properties(libs PROPERTIES FOLDER libs)
set(DEPS dl)
if (NOT ANDROID)
list(APPEND DEPS pthread)
endif()
target_link_libraries(libs ${DEPS} Box2D lua SDL_mixer SDL_image)
