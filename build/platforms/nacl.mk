SO_EXT                    = so
SO_LDFLAGS                = -shared
SO_CFLAGS                 = -fpic
SO_LIBS                  :=

# this in fact disables it
HAVE_SDL_NET_H           ?= 1

EXE_EXT                   = .pexe

NACL_CFG                 ?= $(NACL_SDK_ROOT)/tools/nacl_config.py
NACL_CFLAGS              := $(shell $(NACL_CFG) -t pnacl --cflags)
NACL_LDFLAGS             := $(shell $(NACL_CFG) -t pnacl --lib)

CFLAGS                   += -D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE -DSQLITE_OMIT_LOAD_EXTENSION -DSDL_VIDEO_DRIVER_NACL=1 -DSDL_AUDIO_DRIVER_NACL=1 -DSDL_FILESYSTEM_NACL=1 -DNONETWORK -DHAVE_POW=1 -DHAVE_OPENGLES2=1 -DSDL_VIDEO_OPENGL_ES2=1 -DSDL_VIDEO_RENDER_OGL_ES2=1 -DHAVE_MALLOC=1 -DHAVE_STDIO_H=1 $(NACL_CFLAGS)

LDFLAGS                  += $(NACL_LDFLAGS)/pnacl/Release -lppapi_simple -lnacl_io -lppapi_cpp -lppapi -lpthread -lppapi_gles2
OPENGL_LIBS               =
SDL_NET_SRCS              =

MAKEFILEPATH   = $(CURDIR)/$(lastword $(MAKEFILE_LIST))
NACL_VERSION  ?= 35
NACL_SDK_ROOT ?= $(realpath $(dir $(MAKEFILEPATH))/../nacl_sdk/pepper_$(NACL_VERSION))
CHROME_BIN    ?= google-chrome
NACL_TOOLCHAIN_ROOT ?= $(NACL_SDK_ROOT)/toolchain/$(HOST_OS)_pnacl
