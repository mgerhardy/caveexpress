FILE(GLOB SRCS
	src/video/*.c
	src/video/raspberry/*.c
	src/video/dummy/*.c
	src/video/directfb/*.c
	src/video/pandora/*.c
	src/video/x11/*.c
	src/video/wayland/*.c
	src/video/bwindow/*.c
	src/file/*.c
	src/render/*.c
	src/render/opengl/*.c
	src/render/opengles/*.c
	src/render/software/*.c
	src/render/opengles2/*.c
	src/haptic/*.c
	src/haptic/dummy/*.c
	src/loadso/*.c
	src/loadso/dummy/*.c
	src/loadso/dlopen/*.c
	src/dynapi/*.c
	src/main/*.c
	src/timer/*.c
	src/timer/dummy/*.c
	src/filesystem/*.c
	src/filesystem/unix/*.c
	src/filesystem/dummy/*.c
	src/events/*.c
	src/joystick/*.c
	src/joystick/bsd/*.c
	src/joystick/dummy/*.c
	src/input/*.c
	src/input/evdev/*.c
	src/core/*.c
	src/cpuinfo/*.c
	src/libm/*.c
	src/power/*.c
	src/power/uikit/*.c
	src/audio/*.c
	src/audio/bsd/*.c
	src/audio/esd/*.c
	src/audio/dummy/*.c
	src/audio/dsp/*.c
	src/audio/paudio/*.c
	src/audio/baudio/*.c
	src/audio/sndio/*.c
	src/audio/disk/*.c
	src/audio/sun/*.c
	src/audio/qsa/*.c
	src/audio/fusionsound/*.c
	src/audio/nas/*.c
	src/atomic/*.c
	src/thread/*.c
	src/stdlib/*.c
	src/*.c
	)

if(ANDROID)
FILE(GLOB PLATFORM_SRCS
	src/filesystem/android/*.c
	src/video/android/*.c
	src/core/android/*.c
	src/timer/unix/*.c
	src/main/android/*.c
	src/thread/pthread/*.c
	src/audio/android/*.c
	src/joystick/android/*.c
	src/power/android/*.c)
elseif(EMSCRIPTEN)
FILE(GLOB PLATFORM_SRCS
	src/audio/emscripten/*.c
	src/filesystem/emscripten/*.c
	src/joystick/emscripten/*.c
	src/power/emscripten/*.c
	src/video/emscripten/*.c
	src/thread/generic/*.c
	src/timer/unix/*.c)
elseif(NACL)
FILE(GLOB PLATFORM_SRCS
	src/audio/nacl/*.c
	src/filesystem/nacl/*.c
	src/video/nacl/*.c
	src/timer/unix/*.c
	src/thread/pthread/*.c
	src/main/nacl/*.c)
elseif(DARWIN)
FILE(GLOB PLATFORM_SRCS
	src/video/uikit/*.m
	src/video/cocoa/*.m
	src/file/cocoa/*.m
	src/filesystem/cocoa/*.m
	src/joystick/iphoneos/*.m
	src/power/uikit/*.m
	src/audio/coreaudio/*.c
	src/filesystem/cocoa/*.c
	src/joystick/iphoneos/*.c
	src/joystick/darwin/*.c
	src/timer/unix/*.c
	src/video/uikit/*.c
	src/main/dummy/*.c
	src/video/cocoa/*.c
	src/file/cocoa/*.c
	src/haptic/darwin/*.c
	src/power/macosx/*.c
	src/thread/pthread/*.c)
elseif(WIN32)
FILE(GLOB PLATFORM_SRCS
	src/joystick/windows/*.c
	src/filesystem/windows/*.c
	src/haptic/windows/*.c
	src/video/windows/*.c
	src/loadso/windows/*.c
	src/main/windows/*.c
	src/timer/windows/*.c
	src/core/windows/*.c
	src/power/windows/*.c
	src/audio/winmm/*.c
	src/audio/directsound/*.c
	src/render/direct3d/*.c
	src/thread/windows/*.c
	src/thread/generic/SDL_syscond.c)
elseif(LINUX)
include(FindPkgConfig)
pkg_search_module(DBUS REQUIRED dbus-1 dbus)
include_directories(${DBUS_INCLUDE_DIRS})
FILE(GLOB PLATFORM_SRCS
	src/thread/pthread/*.c
	src/haptic/linux/*.c
	src/joystick/linux/*.c
	src/audio/alsa/*.c
	src/audio/arts/*.c
	src/audio/pulseaudio/*.c
	src/timer/unix/*.c
	src/main/dummy/*.c
	src/core/linux/*.c
	src/power/linux/*.c)
endif()

add_library(SDL2 ${SRCS} ${PLATFORM_SRCS})
if (NOT WINDOWS)
target_link_libraries(SDL2 dl)
endif()
set_target_properties(SDL2 PROPERTIES COMPILE_FLAGS "-Wno-cast-qual -Wno-unused-parameter -Wno-sign-compare")
