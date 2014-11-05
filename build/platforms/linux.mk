SO_EXT                    = so
SO_LDFLAGS                = -shared
SO_CFLAGS                 = -fpic
SO_LIBS                  := -ldl

CFLAGS                   += -D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE

LDFLAGS                  +=
tests_LDFLAGS            += -lrt
ifneq ($(HAVE_SDL_H),1)
tmp_CFLAGS               := $(SDL_CFLAGS)
DBUS_CFLAGS              := $(call PKG_CFLAGS,dbus-1)
SDL_CFLAGS               := -Isrc/libs/SDL/linux $(filter-out -Isrc/libs/SDL/general,$(tmp_CFLAGS)) $(DBUS_CFLAGS)
SDL_LIBS                 += -lpthread -lrt
endif
SQLITE3_LIBS             += -lpthread
