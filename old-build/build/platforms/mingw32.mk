SO_EXT                    = dll
SO_LDFLAGS                = -shared
SO_CFLAGS                 = -fpic
SO_LIBS                  :=
EXE_EXT                  := .exe

CFLAGS                   += -DWINVER=0x501
LDFLAGS                  +=

OPENGL_LIBS              ?= -lopengl32
SDL_NET_LIBS             += -lws2_32 -liphlpapi
$(APPNAME)_LDFLAGS       ?= -static-libgcc -static-libstdc++

ifneq ($(HAVE_SDL_H),1)
LDFLAGS                  += -lmingw32 -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid
endif
