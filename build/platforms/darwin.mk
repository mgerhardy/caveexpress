SO_EXT                    = dylib
SO_CFLAGS                 = -fPIC -fno-common
SO_LDFLAGS                = -dynamiclib
SO_LIBS                  := -ldl

CFLAGS                   += -D_BSD_SOURCE -D_XOPEN_SOURCE
LDFLAGS                  += -framework IOKit -framework Foundation -framework Cocoa
LDFLAGS                  += -rdynamic

CXXFLAGS                 += -std=c++11

### most mac users will have their additional libs and headers under $(SDKROOT),
### check for that, and if present, add to CFLAGS/LDFLAGS (really convenient!)
SDKROOT                  ?=
FRAMEWORK_DIR            ?= $(SDKROOT)/Library/Frameworks

CFLAGS                   += -I$(SDKROOT)/include -F$(FRAMEWORK_DIR) -mmacosx-version-min=10.4
LDFLAGS                  += -L$(SDKROOT)/lib -F$(FRAMEWORK_DIR) -mmacosx-version-min=10.4

ifdef UNIVERSAL
	CFLAGS += -arch i386 -arch ppc
	LDFLAGS += -arch i386 -arch ppc
endif

OPENGL_CFLAGS            ?=
OPENGL_LIBS              ?= -framework OpenGL

SDL_IMAGE_SRCS           += \
	libs/SDL_image/IMG_ImageIO.m \
	src/libs/SDL_image/IMG_UIImage.m
