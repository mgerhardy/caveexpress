SO_EXT                    = dylib
SO_CFLAGS                 = -fPIC -fno-common
SO_LDFLAGS                = -dynamiclib
SO_LIBS                  := -ldl

SDKVERSION               ?=
SDKROOT                  ?=

IOSROOT                  ?= ./ios
FRAMEWORK_DIR            ?= $(IOSROOT)/frameworks
PKG_CONFIG_PATH          ?= $(SDKROOT)/usr/lib/pkgconfig

LIBS_DIR                 ?= $(IOSROOT)/lib
INCLUDE_DIR              ?= $(IOSROOT)/include

CFLAGS                   += -isysroot $(SDKROOT) -I$(INCLUDE_DIR) -F$(FRAMEWORK_DIR)
LDFLAGS                  += -isysroot $(SDKROOT) -Wl,-syslibroot $(SDKROOT) -L$(LIBS_DIR) -F$(FRAMEWORK_DIR)

SDL_CFLAGS               ?= -I$(FRAMEWORK_DIR)/SDL.framework/Headers
SDL_LIBS                 ?= -framework CoreFoundation -framework Foundation -framework CoreGraphics -framework UIKit -framework CoreAudio -framework AudioToolbox -framework Quartzcore -framework SDL
OPENGL_CFLAGS            ?=
OPENGL_LIBS              ?= -framework OpenGLES

SDL_IMAGE_SRCS           += \
	libs/SDL_image/IMG_ImageIO.m \
	src/libs/SDL_image/IMG_UIImage.m
