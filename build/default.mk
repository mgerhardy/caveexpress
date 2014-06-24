PKG_CONFIG               ?= PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(CROSS)pkg-config
ifeq ($(HAVE_SDL_H),1)
SDL_LIBS                 += $(call PKG_LIBS,sdl2)
SDL_CFLAGS               ?= $(call PKG_CFLAGS,sdl2)
SDL_SRCS                 :=
else
SDL_LIBS                 +=
SDL_CFLAGS               ?= -Isrc/libs/SDL/general -Isrc/libs/SDL/include -Isrc/libs/SDL/src
SDL_SRCS                  = \
	$(subst src/libs/SDL/,libs/SDL/, \
	$(wildcard src/libs/SDL/src/video/*.c) \
	$(wildcard src/libs/SDL/src/video/raspberry/*.c) \
	$(wildcard src/libs/SDL/src/video/dummy/*.c) \
	$(wildcard src/libs/SDL/src/video/directfb/*.c) \
	$(wildcard src/libs/SDL/src/video/pandora/*.c) \
	$(wildcard src/libs/SDL/src/video/x11/*.c) \
	$(wildcard src/libs/SDL/src/video/wayland/*.c) \
	$(wildcard src/libs/SDL/src/video/bwindow/*.c) \
	$(wildcard src/libs/SDL/src/file/*.c) \
	$(wildcard src/libs/SDL/src/render/*.c) \
	$(wildcard src/libs/SDL/src/render/opengl/*.c) \
	$(wildcard src/libs/SDL/src/render/opengles/*.c) \
	$(wildcard src/libs/SDL/src/render/direct3d/*.c) \
	$(wildcard src/libs/SDL/src/render/software/*.c) \
	$(wildcard src/libs/SDL/src/render/opengles2/*.c) \
	$(wildcard src/libs/SDL/src/haptic/*.c) \
	$(wildcard src/libs/SDL/src/haptic/dummy/*.c) \
	$(wildcard src/libs/SDL/src/loadso/*.c) \
	$(wildcard src/libs/SDL/src/loadso/dummy/*.c) \
	$(wildcard src/libs/SDL/src/loadso/dlopen/*.c) \
	$(wildcard src/libs/SDL/src/dynapi/*.c) \
	$(wildcard src/libs/SDL/src/main/*.c) \
	$(wildcard src/libs/SDL/src/timer/*.c) \
	$(wildcard src/libs/SDL/src/timer/unix/*.c) \
	$(wildcard src/libs/SDL/src/timer/dummy/*.c) \
	$(wildcard src/libs/SDL/src/filesystem/*.c) \
	$(wildcard src/libs/SDL/src/filesystem/unix/*.c) \
	$(wildcard src/libs/SDL/src/filesystem/dummy/*.c) \
	$(wildcard src/libs/SDL/src/events/*.c) \
	$(wildcard src/libs/SDL/src/joystick/*.c) \
	$(wildcard src/libs/SDL/src/joystick/bsd/*.c) \
	$(wildcard src/libs/SDL/src/joystick/dummy/*.c) \
	$(wildcard src/libs/SDL/src/input/*.c) \
	$(wildcard src/libs/SDL/src/input/evdev/*.c) \
	$(wildcard src/libs/SDL/src/core/*.c) \
	$(wildcard src/libs/SDL/src/cpuinfo/*.c) \
	$(wildcard src/libs/SDL/src/libm/*.c) \
	$(wildcard src/libs/SDL/src/power/*.c) \
	$(wildcard src/libs/SDL/src/power/uikit/*.c) \
	$(wildcard src/libs/SDL/src/audio/*.c) \
	$(wildcard src/libs/SDL/src/audio/bsd/*.c) \
	$(wildcard src/libs/SDL/src/audio/arts/*.c) \
	$(wildcard src/libs/SDL/src/audio/esd/*.c) \
	$(wildcard src/libs/SDL/src/audio/dummy/*.c) \
	$(wildcard src/libs/SDL/src/audio/dsp/*.c) \
	$(wildcard src/libs/SDL/src/audio/paudio/*.c) \
	$(wildcard src/libs/SDL/src/audio/baudio/*.c) \
	$(wildcard src/libs/SDL/src/audio/sndio/*.c) \
	$(wildcard src/libs/SDL/src/audio/disk/*.c) \
	$(wildcard src/libs/SDL/src/audio/sun/*.c) \
	$(wildcard src/libs/SDL/src/audio/qsa/*.c) \
	$(wildcard src/libs/SDL/src/audio/alsa/*.c) \
	$(wildcard src/libs/SDL/src/audio/fusionsound/*.c) \
	$(wildcard src/libs/SDL/src/audio/pulseaudio/*.c) \
	$(wildcard src/libs/SDL/src/audio/nas/*.c) \
	$(wildcard src/libs/SDL/src/audio/winmm/*.c) \
	$(wildcard src/libs/SDL/src/audio/directsound/*.c) \
	$(wildcard src/libs/SDL/src/atomic/*.c) \
	$(wildcard src/libs/SDL/src/thread/*.c) \
	$(wildcard src/libs/SDL/src/stdlib/*.c) \
	$(wildcard src/libs/SDL/src/*.c) \
	)
ifeq ($(TARGET_OS),darwin)
	SDL_SRCS += \
		$(subst src/libs/SDL/,libs/SDL/, \
		$(wildcard src/libs/SDL/src/video/uikit/*.m) \
		$(wildcard src/libs/SDL/src/video/cocoa/*.m) \
		$(wildcard src/libs/SDL/src/file/cocoa/*.m) \
		$(wildcard src/libs/SDL/src/filesystem/cocoa/*.m) \
		$(wildcard src/libs/SDL/src/joystick/iphoneos/*.m) \
		$(wildcard src/libs/SDL/src/power/uikit/*.m) \
		$(wildcard src/libs/SDL/src/audio/coreaudio/*.c) \
		$(wildcard src/libs/SDL/src/filesystem/cocoa/*.c) \
		$(wildcard src/libs/SDL/src/joystick/iphoneos/*.c) \
		$(wildcard src/libs/SDL/src/joystick/darwin/*.c) \
		$(wildcard src/libs/SDL/src/video/uikit/*.c) \
		$(wildcard src/libs/SDL/src/main/dummy/*.c) \
		$(wildcard src/libs/SDL/src/video/cocoa/*.c) \
		$(wildcard src/libs/SDL/src/file/cocoa/*.c) \
		$(wildcard src/libs/SDL/src/haptic/darwin/*.c) \
		$(wildcard src/libs/SDL/src/power/macosx/*.c) \
		$(wildcard src/libs/SDL/src/thread/pthread/*.c) \
		)
endif
ifneq ($(findstring $(TARGET_OS), mingw32 mingw64 mingw64_64),)
	SDL_SRCS += \
		$(subst src/libs/SDL/,libs/SDL/, \
		$(wildcard src/libs/SDL/src/joystick/windows/*.c) \
		$(wildcard src/libs/SDL/src/filesystem/windows/*.c) \
		$(wildcard src/libs/SDL/src/haptic/windows/*.c) \
		$(wildcard src/libs/SDL/src/video/windows/*.c) \
		$(wildcard src/libs/SDL/src/loadso/windows/*.c) \
		$(wildcard src/libs/SDL/src/main/windows/*.c) \
		$(wildcard src/libs/SDL/src/timer/windows/*.c) \
		$(wildcard src/libs/SDL/src/core/windows/*.c) \
		$(wildcard src/libs/SDL/src/power/windows/*.c) \
		$(wildcard src/libs/SDL/src/thread/windows/*.c) \
		$(wildcard src/libs/SDL/src/thread/generic/SDL_syscond.c) \
		)
endif
ifneq ($(findstring $(TARGET_OS), netbsd freebsd linux),)
	SDL_SRCS += \
		$(subst src/libs/SDL/,libs/SDL/, \
			$(wildcard src/libs/SDL/src/thread/pthread/*.c) \
			$(wildcard src/libs/SDL/src/haptic/linux/*.c) \
			$(wildcard src/libs/SDL/src/joystick/linux/*.c) \
			$(wildcard src/libs/SDL/src/main/dummy/*.c) \
			$(wildcard src/libs/SDL/src/core/linux/*.c) \
			$(wildcard src/libs/SDL/src/power/linux/*.c) \
		)
endif
ifeq ($(TARGET_OS),nacl)
	SDL_SRCS += \
		$(subst src/libs/SDL/,libs/SDL/, \
			$(wildcard src/libs/SDL/src/audio/nacl/*.c) \
			$(wildcard src/libs/SDL/src/filesystem/nacl/*.c) \
			$(wildcard src/libs/SDL/src/video/nacl/*.c) \
			$(wildcard src/libs/SDL/src/timer/unix/*.c) \
			$(wildcard src/libs/SDL/src/thread/pthread/*.c) \
			$(wildcard src/libs/SDL/src/main/nacl/*.c) \
		)
endif
endif
ifeq ($(HAVE_SDL_RWHTTP_H),1)
SDL_RWHTTP_LIBS          += $(call PKG_LIBS,SDL_rwhttp)
SDL_RWHTTP_CFLAGS        ?= $(call PKG_CFLAGS,SDL_rwhttp)
else
SDL_RWHTTP_LIBS          +=
SDL_RWHTTP_CFLAGS        ?=
endif
ifeq ($(HAVE_SDL_IMAGE_H),1)
SDL_IMAGE_LIBS           += $(call PKG_LIBS,SDL2_image)
SDL_IMAGE_CFLAGS         ?= $(call PKG_CFLAGS,SDL2_image)
SDL_IMAGE_SRCS            =
PNG_CFLAGS               ?= $(call PKG_CFLAGS,png,png)
PNG_LIBS                 += $(call PKG_LIBS,png,png)
else
SDL_IMAGE_LIBS           +=
SDL_IMAGE_CFLAGS         ?= -DLOAD_PNG -Isrc/libs/SDL_image -Isrc/libs/libpng-1.6.2 -DPNG_NO_CONFIG_H -DHAVE_SDL_IMAGE_H
SDL_IMAGE_SRCS            = \
	libs/libpng-1.6.2/png.c \
	libs/libpng-1.6.2/pngerror.c \
	libs/libpng-1.6.2/pngget.c \
	libs/libpng-1.6.2/pngmem.c \
	libs/libpng-1.6.2/pngpread.c \
	libs/libpng-1.6.2/pngread.c \
	libs/libpng-1.6.2/pngrio.c \
	libs/libpng-1.6.2/pngrtran.c \
	libs/libpng-1.6.2/pngrutil.c \
	libs/libpng-1.6.2/pngset.c \
	libs/libpng-1.6.2/pngtrans.c \
	libs/libpng-1.6.2/pngwio.c \
	libs/libpng-1.6.2/pngwrite.c \
	libs/libpng-1.6.2/pngwtran.c \
	libs/libpng-1.6.2/pngwutil.c \
	libs/SDL_image/IMG_bmp.c \
	libs/SDL_image/IMG.c \
	libs/SDL_image/IMG_gif.c \
	libs/SDL_image/IMG_jpg.c \
	libs/SDL_image/IMG_lbm.c \
	libs/SDL_image/IMG_pcx.c \
	libs/SDL_image/IMG_png.c \
	libs/SDL_image/IMG_pnm.c \
	libs/SDL_image/IMG_tga.c \
	libs/SDL_image/IMG_tif.c \
	libs/SDL_image/IMG_webp.c \
	libs/SDL_image/IMG_xcf.c \
	libs/SDL_image/IMG_xpm.c \
	libs/SDL_image/IMG_xv.c
endif
ifeq ($(HAVE_SDL_MIXER_H),1)
SDL_MIXER_LIBS           += $(call PKG_LIBS,SDL2_mixer)
SDL_MIXER_CFLAGS         ?= $(call PKG_CFLAGS,SDL2_mixer)
SDL_MIXER_SRCS            =
VORBIS_CFLAGS            ?= $(call PKG_CFLAGS,vorbis) $(call PKG_CFLAGS,vorbisfile)
VORBIS_LIBS              += $(call PKG_LIBS,vorbis) $(call PKG_LIBS,vorbisfile)
OGG_CFLAGS               ?= $(call PKG_CFLAGS,ogg)
OGG_LIBS                 += $(call PKG_LIBS,ogg)
else
SDL_MIXER_LIBS           +=
SDL_MIXER_CFLAGS         ?= -Isrc/libs/libogg-1.3.1/include -Isrc/libs/SDL_mixer -DOGG_MUSIC -DWAV_MUSIC -DHAVE_SDL_MIXER_H
TREMOR_CFLAGS             = -Isrc/libs/libvorbisidec-1.2.1 -DOGG_USE_TREMOR
TREMOR_SRCS               = \
	libs/libvorbisidec-1.2.1/mdct.c \
	libs/libvorbisidec-1.2.1/block.c \
	libs/libvorbisidec-1.2.1/window.c \
	libs/libvorbisidec-1.2.1/synthesis.c \
	libs/libvorbisidec-1.2.1/info.c \
	libs/libvorbisidec-1.2.1/floor1.c \
	libs/libvorbisidec-1.2.1/floor0.c \
	libs/libvorbisidec-1.2.1/vorbisfile.c \
	libs/libvorbisidec-1.2.1/res012.c \
	libs/libvorbisidec-1.2.1/mapping0.c \
	libs/libvorbisidec-1.2.1/registry.c \
	libs/libvorbisidec-1.2.1/codebook.c \
	libs/libvorbisidec-1.2.1/sharedbook.c
VORBIS_CFLAGS             = -Isrc/libs/libvorbis-1.3.3/include/vorbis -Isrc/libs/libvorbis-1.3.3/include -Isrc/libs/libvorbis-1.3.3/lib
VORBIS_SRCS               = \
	libs/libvorbis-1.3.3/lib/mapping0.c \
	libs/libvorbis-1.3.3/lib/synthesis.c \
	libs/libvorbis-1.3.3/lib/mdct.c \
	libs/libvorbis-1.3.3/lib/lpc.c \
	libs/libvorbis-1.3.3/lib/lookup.c \
	libs/libvorbis-1.3.3/lib/info.c \
	libs/libvorbis-1.3.3/lib/block.c \
	libs/libvorbis-1.3.3/lib/lsp.c \
	libs/libvorbis-1.3.3/lib/vorbisfile.c \
	libs/libvorbis-1.3.3/lib/envelope.c \
	libs/libvorbis-1.3.3/lib/registry.c \
	libs/libvorbis-1.3.3/lib/smallft.c \
	libs/libvorbis-1.3.3/lib/bitrate.c \
	libs/libvorbis-1.3.3/lib/sharedbook.c \
	libs/libvorbis-1.3.3/lib/window.c \
	libs/libvorbis-1.3.3/lib/res0.c \
	libs/libvorbis-1.3.3/lib/codebook.c \
	libs/libvorbis-1.3.3/lib/vorbisenc.c \
	libs/libvorbis-1.3.3/lib/psy.c \
	libs/libvorbis-1.3.3/lib/analysis.c \
	libs/libvorbis-1.3.3/lib/floor0.c \
	libs/libvorbis-1.3.3/lib/floor1.c
SDL_MIXER_SRCS            = \
	libs/libogg-1.3.1/src/framing.c \
	libs/libogg-1.3.1/src/bitwise.c \
	libs/SDL_mixer/dynamic_flac.c \
	libs/SDL_mixer/dynamic_fluidsynth.c \
	libs/SDL_mixer/dynamic_mod.c \
	libs/SDL_mixer/dynamic_mp3.c \
	libs/SDL_mixer/dynamic_ogg.c \
	libs/SDL_mixer/effect_position.c \
	libs/SDL_mixer/effects_internal.c \
	libs/SDL_mixer/effect_stereoreverse.c \
	libs/SDL_mixer/fluidsynth.c \
	libs/SDL_mixer/load_aiff.c \
	libs/SDL_mixer/load_flac.c \
	libs/SDL_mixer/load_ogg.c \
	libs/SDL_mixer/load_voc.c \
	libs/SDL_mixer/mixer.c \
	libs/SDL_mixer/music.c \
	libs/SDL_mixer/music_cmd.c \
	libs/SDL_mixer/music_flac.c \
	libs/SDL_mixer/music_mad.c \
	libs/SDL_mixer/music_mod.c \
	libs/SDL_mixer/music_modplug.c \
	libs/SDL_mixer/music_ogg.c \
	libs/SDL_mixer/wavestream.c

ifneq ($(findstring $(TARGET_OS), android ouya),)
	SDL_MIXER_CFLAGS += $(TREMOR_CFLAGS)
	SDL_MIXER_SRCS   += $(TREMOR_SRCS)
else
	SDL_MIXER_CFLAGS += $(VORBIS_CFLAGS)
	SDL_MIXER_SRCS   += $(VORBIS_SRCS)
endif

endif
OPENGLES_CFLAGS          ?= $(call PKG_CFLAGS,glesv2) $(call PKG_CFLAGS,egl)
OPENGLES_LIBS            += $(call PKG_LIBS,glesv2) $(call PKG_LIBS,egl)
OPENGL_CFLAGS            ?= $(call PKG_CFLAGS,gl,gl)
OPENGL_LIBS              += $(call PKG_LIBS,gl,gl)
GTEST_CFLAGS             ?= -Isrc/libs/gtest/include -Isrc/libs/gtest
GTEST_LIBS               +=
GTEST_SRCS                = libs/gtest/src/gtest-all.cc
ifeq ($(HAVE_NCURSES_H),1)
NCURSES_CFLAGS           ?= $(call PKG_CFLAGS,ncurses)
NCURSES_LIBS             += $(call PKG_LIBS,ncurses)
endif
LUA_SRCS                  = \
	libs/lua/lapi.cpp \
	libs/lua/lauxlib.cpp \
	libs/lua/lbaselib.cpp \
	libs/lua/lbitlib.cpp \
	libs/lua/lcode.cpp \
	libs/lua/lcorolib.cpp \
	libs/lua/lctype.cpp \
	libs/lua/ldblib.cpp \
	libs/lua/ldebug.cpp \
	libs/lua/ldo.cpp \
	libs/lua/ldump.cpp \
	libs/lua/lfunc.cpp \
	libs/lua/lgc.cpp \
	libs/lua/linit.cpp \
	libs/lua/liolib.cpp \
	libs/lua/llex.cpp \
	libs/lua/lmathlib.cpp \
	libs/lua/lmem.cpp \
	libs/lua/loadlib.cpp \
	libs/lua/lobject.cpp \
	libs/lua/lopcodes.cpp \
	libs/lua/loslib.cpp \
	libs/lua/lparser.cpp \
	libs/lua/lstate.cpp \
	libs/lua/lstring.cpp \
	libs/lua/lstrlib.cpp \
	libs/lua/ltable.cpp \
	libs/lua/ltablib.cpp \
	libs/lua/ltm.cpp \
	libs/lua/lundump.cpp \
	libs/lua/lvm.cpp \
	libs/lua/lzio.cpp
LUA_CFLAGS               ?= -Isrc/libs/lua -DLUA_USE_LONGJMP -DLUA_COMPAT_MODULE
LUA_LIBS                 +=
ifeq ($(HAVE_SQLITE3_H),1)
SQLITE3_SRCS              =
SQLITE3_CFLAGS           ?= $(call PKG_CFLAGS,sqlite3)
SQLITE3_LIBS             += $(call PKG_LIBS,sqlite3)
else
ifeq ($(TARGET_OS),html5)
SQLITE3_SRCS              =
SQLITE3_CFLAGS           ?=
else
SQLITE3_SRCS              = libs/sqlite/sqlite3.c
SQLITE3_CFLAGS           ?= -Isrc/libs/sqlite -DSQLITE_HAVE_ISNAN
endif
SQLITE3_LIBS             += -lpthread
endif
ifeq ($(HAVE_TINYXML2_H),1)
TINYXML2_SRCS             =
TINYXML2_CFLAGS          ?= $(call PKG_CFLAGS,tinyxml2)
TINYXML2_LIBS            ?= $(call PKG_LIBS,tinyxml2)
else
TINYXML2_SRCS             = libs/tinyxml2/tinyxml2.cpp
TINYXML2_CFLAGS          ?= -Isrc/libs/tinyxml2
TINYXML2_LIBS            +=
endif
ifeq ($(HAVE_SDL_NET_H),1)
SDL_NET_SRCS              =
SDL_NET_CFLAGS           ?= $(call PKG_CFLAGS,SDL2_net)
SDL_NET_LIBS             += $(call PKG_LIBS,SDL2_net)
else
SDL_NET_SRCS              = \
	libs/SDL_net/SDLnet.c \
	libs/SDL_net/SDLnetselect.c \
	libs/SDL_net/SDLnetTCP.c \
	libs/SDL_net/SDLnetUDP.c
SDL_NET_CFLAGS           ?= -Isrc/libs/SDL_net
SDL_NET_LIBS             +=
endif
YAJL_SRCS                 = \
	libs/yajl/yajl_alloc.c \
	libs/yajl/yajl_buf.c \
	libs/yajl/yajl.c \
	libs/yajl/yajl_encode.c \
	libs/yajl/yajl_gen.c \
	libs/yajl/yajl_lex.c \
	libs/yajl/yajl_parser.c \
	libs/yajl/yajl_tree.c
YAJL_CFLAGS              ?= -Isrc/libs/yajl
YAJL_LIBS                +=
ifeq ($(HAVE_ZLIB_H),1)
ZLIB_SRCS                 =
ZLIB_CFLAGS               =
ZLIB_LIBS                 = -lz
else
ZLIB_SRCS                 = \
	libs/zlib/adler32.c \
	libs/zlib/compress.c \
	libs/zlib/crc32.c \
	libs/zlib/deflate.c \
	libs/zlib/gzclose.c \
	libs/zlib/gzlib.c \
	libs/zlib/gzread.c \
	libs/zlib/gzwrite.c \
	libs/zlib/infback.c \
	libs/zlib/inffast.c \
	libs/zlib/inflate.c \
	libs/zlib/inftrees.c \
	libs/zlib/trees.c \
	libs/zlib/uncompr.c \
	libs/zlib/zutil.c
ZLIB_CFLAGS              ?= -Isrc/libs/zlib
ZLIB_LIBS                 =
endif
