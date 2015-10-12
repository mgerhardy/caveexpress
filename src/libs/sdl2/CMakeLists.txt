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
	src/dynapi/*.c
	src/main/*.c
	src/timer/*.c
	src/filesystem/*.c
	src/events/*.c
	src/joystick/*.c
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
	src/filesystem/unix/*.c
	src/filesystem/dummy/*.c
	src/filesystem/android/*.c
	src/video/android/*.c
	src/core/android/*.c
	src/timer/unix/*.c
	src/loadso/dlopen/*.c
	src/main/android/*.c
	src/thread/pthread/*.c
	src/audio/android/*.c
	src/joystick/dummy/*.c
	src/joystick/android/*.c
	src/power/android/*.c)
elseif(EMSCRIPTEN)
FILE(GLOB PLATFORM_SRCS
	src/audio/emscripten/*.c
	src/filesystem/emscripten/*.c
	src/joystick/emscripten/*.c
	src/loadso/dlopen/*.c
	src/power/emscripten/*.c
	src/video/emscripten/*.c
	src/thread/generic/*.c
	src/timer/unix/*.c)
elseif(NACL)
FILE(GLOB PLATFORM_SRCS
	src/audio/nacl/*.c
	src/filesystem/nacl/*.c
	src/video/nacl/*.c
	src/joystick/dummy/*.c
	src/loadso/dummy/*.c
	src/loadso/dlopen/*.c
	src/timer/unix/*.c
	src/thread/pthread/*.c
	src/main/nacl/*.c)
elseif(IOS)
FILE(GLOB PLATFORM_SRCS
	src/video/uikit/*.m
	src/video/cocoa/*.m
	src/file/cocoa/*.m
	src/loadso/dummy/*.c
	src/loadso/dlopen/*.c
	src/filesystem/cocoa/*.m
	src/joystick/iphoneos/*.m
	src/power/uikit/*.m
	src/audio/coreaudio/*.c
	src/filesystem/cocoa/*.c
	src/joystick/darwin/*.c
	src/timer/unix/*.c
	src/video/uikit/*.c
	src/main/dummy/*.c
	src/video/cocoa/*.c
	src/file/cocoa/*.c
	src/haptic/darwin/*.c
	src/power/macosx/*.c
	src/thread/pthread/*.c)
elseif(DARWIN)
FILE(GLOB PLATFORM_SRCS
	src/video/uikit/*.m
	src/video/cocoa/*.m
	src/file/cocoa/*.m
	src/filesystem/cocoa/*.m
	src/loadso/dummy/*.c
	src/loadso/dlopen/*.c
	src/power/uikit/*.m
	src/audio/coreaudio/*.c
	src/filesystem/cocoa/*.c
	src/joystick/darwin/*.c
	src/timer/unix/*.c
	src/video/uikit/*.c
	src/main/dummy/*.c
	src/video/cocoa/*.c
	src/file/cocoa/*.c
	src/haptic/darwin/*.c
	src/power/macosx/*.c
	src/thread/pthread/*.c)
elseif(WINDOWS)
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
	src/audio/xaudio2/*.c
	src/audio/directsound/*.c
	src/render/direct3d/*.c
	src/thread/windows/*.c
	src/thread/generic/SDL_syscond.c)
elseif(LINUX)
FILE(GLOB PLATFORM_SRCS
	src/thread/pthread/*.c
	src/haptic/linux/*.c
	src/joystick/bsd/*.c
	src/joystick/dummy/*.c
	src/joystick/linux/*.c
	src/filesystem/unix/*.c
	src/filesystem/dummy/*.c
	src/loadso/dlopen/*.c
	src/audio/alsa/*.c
	src/audio/arts/*.c
	src/audio/pulseaudio/*.c
	src/timer/unix/*.c
	src/main/dummy/*.c
	src/core/linux/*.c
	src/power/linux/*.c)
endif()

if (ANDROID)
	set(ANDROID_LDFLAGS "-Wl,--undefined=Java_org_libsdl_app_SDLActivity_nativeInit")
endif()

cp_add_library(
	LIB sdl2
	SRCS ${SRCS} ${PLATFORM_SRCS}
	CFLAGS "-Wno-cast-qual -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-discarded-qualifiers -Wno-empty-body -Wno-unused-but-set-variable"
	LINKERFLAGS ${ANDROID_LDFLAGS})
if (NOT SDL2_FOUND)
	if (CMAKE_DL_LIBS)
		target_link_libraries(sdl2 ${CMAKE_DL_LIBS})
	endif()
	if (IOS)
		set_property(TARGET sdl2 APPEND_STRING PROPERTY COMPILE_FLAGS " -fobjc-arc")
	endif()
	list(APPEND SDL2_INCLUDE_DIRS "${ROOT_DIR}/src/libs/sdl2/include")
	list(APPEND SDL2_INCLUDE_DIRS "${ROOT_DIR}/src/libs/sdl2/src")
	if (ANDROID)
		add_definitions(-DHAVE_STDIO_H=1)
	endif()
	include_directories("${ROOT_DIR}/src/libs/sdl2/src" "${ROOT_DIR}/src/libs/sdl2/include")
	if (LINUX AND NOT ANDROID AND NOT EMSCRIPTEN)
		include(FindPkgConfig)
		pkg_search_module(DBUS dbus-1 dbus)
		if (DBUS_FOUND)
			include_directories(${DBUS_INCLUDE_DIRS})
		endif()
		list(APPEND SDL2_INCLUDE_DIRS "${ROOT_DIR}/src/libs/sdl2/linux")
		include_directories("${ROOT_DIR}/src/libs/sdl2/linux")
		message(STATUS "Use preconfigured SDL for linux")
	else()
		list(APPEND SDL2_INCLUDE_DIRS "${ROOT_DIR}/src/libs/sdl2/general")
		include_directories("${ROOT_DIR}/src/libs/sdl2/general")
	endif()
	var_global(SDL2_INCLUDE_DIRS)
endif()
