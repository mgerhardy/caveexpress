SO_EXT                    = dll
SO_LDFLAGS                = -shared
SO_CFLAGS                 = -fpic
SO_LIBS                  :=
EXE_EXT                  := .exe

CFLAGS                   += -DWINVER=0x501
LDFLAGS                  +=

OPENGL_LIBS              ?= -lopengl64
$(APPNAME)_LDFLAGS       += -lws2_64 -liphlpapi -static-libgcc -static-libstdc++

ifneq ($(HAVE_SDL_H),1)
LDFLAGS                  += -lmingw32 -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser64 -lgdi64 -lwinmm -limm64 -lole64 -loleaut64 -lshell64 -lversion -luuid
endif
