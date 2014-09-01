TARGET             := geophoto

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(SDL_LIBS) $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(OPENGL_LIBS) $(OGG_LIBS) $(VORBIS_LIBS) $(PNG_LIBS) $(NCURSES_LIBS) -lz $(SO_LIBS) $(LUA_LIBS) $(SQLITE3_LIBS) $(TINYXML2_LIBS) $(SDL_NET_LIBS) $(SDL_RWHTTP_LIBS) $(YAJL_LIBS)
$(TARGET)_CFLAGS   += $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) $(SDL_MIXER_CFLAGS) $(OPENGLES_CFLAGS) $(OGG_CFLAGS) $(VORBIS_CFLAGS) $(LUA_CFLAGS) $(PNG_CFLAGS) $(NCURSES_CFLAGS) $(SQLITE3_CFLAGS) $(TINYXML2_CFLAGS) $(SDL_NET_CFLAGS) $(SDL_RWHTTP_CFLAGS) $(YAJL_CFLAGS)
$(TARGET)_SRCS      = $(subst $(SRCDIR)/,, \
	$(wildcard $(SRCDIR)/*.cpp) \
	\
	$(wildcard $(SRCDIR)/common/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/sprites/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/textures/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/sound/sdl/*.cpp) \
	$(wildcard $(SRCDIR)/client/sound/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/round/*.cpp) \
	$(wildcard $(SRCDIR)/client/round//persister/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/ui/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/ui/nodes/*.cpp) \
	\
	$(wildcard $(SRCDIR)/client/ui/windows/*.cpp) \
	\
	$(wildcard $(SRCDIR)/shared/*.cpp) \
	\
	$(wildcard $(SRCDIR)/shared/network/*.cpp) \
	\
	$(wildcard $(SRCDIR)/server/*.cpp) ) \
	\
	$(SDL_NET_SRCS) \
	\
	$(SQLITE3_SRCS) \
	\
	$(LUA_SRCS) \
	\
	$(TINYXML2_SRCS) \
	\
	$(SDL_IMAGE_SRCS) \
	\
	$(SDL_MIXER_SRCS) \
	\
	$(SDL_RWHTTP_SRCS) \
	\
	$(YAJL_SRCS)

ifneq ($(findstring $(TARGET_OS), mingw32 mingw64 mingw64_64),)
	$(TARGET)_SRCS +=\
		common/ports/Windows.cpp \
		common/ports/project.rc
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), netbsd freebsd linux),)
	$(TARGET)_SRCS +=\
		common/ports/Unix.cpp
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), html5),)
	$(TARGET)_SRCS +=\
		common/ports/Unix.cpp \
		common/ports/HTML5.cpp
	$(TARGET)_LDFLAGS +=
endif

ifeq ($(TARGET_OS),darwin)
	$(TARGET)_SRCS +=\
		common/ports/Unix.cpp \
		common/ports/Darwin.cpp \
		common/ports/Cocoa.mm \
	$(TARGET)_LDFLAGS +=
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS) -fno-rtti -fno-exceptions
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
ifeq ($(STATIC_SDL),1)
$(TARGET)_LDFLAGS += -ldl -lrt -lpthread /usr/lib/libSDL2.a
endif
