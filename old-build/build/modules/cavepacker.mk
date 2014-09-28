TARGET             := cavepacker

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(SDL_LIBS) $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(ZLIB_LIBS) $(OPENGL_LIBS) $(OGG_LIBS) $(VORBIS_LIBS) $(PNG_LIBS) $(SO_LIBS) $(LUA_LIBS) $(SQLITE3_LIBS) $(SDL_NET_LIBS) $(SDL_RWHTTP_LIBS) $(NCURSES_LIBS)
$(TARGET)_CFLAGS   += -I$(SRCDIR)/$(TARGET) $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) $(SDL_MIXER_CFLAGS) $(ZLIB_CFLAGS) $(OPENGL_CFLAGS) $(OGG_CFLAGS) $(VORBIS_CFLAGS) $(LUA_CFLAGS) $(PNG_CFLAGS) $(SQLITE3_CFLAGS) $(SDL_NET_CFLAGS) $(SDL_RWHTTP_CFLAGS) $(NCURSES_CFLAGS)
$(TARGET)_SRCS      = $(subst $(SRCDIR)/,, \
	$(wildcard $(SRCDIR)/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/client/shaders/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sprites/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/particles/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/textures/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/entities/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sound/sdl/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sound/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/nodes/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/windows/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/windows/intro/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/layouts/*.cpp) \
	$(wildcard $(SRCDIR)/engine/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/campaign/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/campaign/persister/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/network/*.cpp) \
	$(wildcard $(SRCDIR)/engine/server/*.cpp) \	\
	\
	$(wildcard $(SRCDIR)/cavepacker/client/ui/windows/*.cpp) \
	$(wildcard $(SRCDIR)/cavepacker/client/ui/windows/intro/*.cpp) \
	$(wildcard $(SRCDIR)/cavepacker/client/ui/nodes/*.cpp) \
	\
	$(wildcard $(SRCDIR)/cavepacker/client/*.cpp) \
	\
	$(wildcard $(SRCDIR)/cavepacker/server/*.cpp) \
	$(wildcard $(SRCDIR)/cavepacker/server/entities/*.cpp) \
	$(wildcard $(SRCDIR)/cavepacker/server/map/*.cpp) \
	\
	$(wildcard $(SRCDIR)/cavepacker/shared/*.cpp) \
	$(wildcard $(SRCDIR)/cavepacker/shared/network/*.cpp) \
	\
	$(wildcard $(SRCDIR)/cavepacker/*.cpp) \
	) \
	\
	$(SQLITE3_SRCS) \
	\
	$(LUA_SRCS) \
	\
	$(ZLIB_SRCS) \
	\
	$(SDL_SRCS) \
	\
	$(SDL_IMAGE_SRCS) \
	\
	$(SDL_MIXER_SRCS)

ifneq ($(findstring $(TARGET_OS), mingw32 mingw64),)
	$(TARGET)_SRCS +=\
		$(SDL_NET_SRCS) \
		engine/common/ports/Windows.cpp \
		engine/common/ports/project.rc
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), netbsd freebsd linux),)
	$(TARGET)_SRCS +=\
		$(SDL_NET_SRCS) \
		engine/common/ports/Unix.cpp
	$(TARGET)_LDFLAGS +=
endif

ifeq ($(TARGET_OS),nacl)
	$(TARGET)_SRCS +=\
		$(SDL_NET_SRCS) \
		engine/common/ports/NaCl.cpp
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), html5),)
	$(TARGET)_SRCS +=\
		engine/common/ports/Unix.cpp \
		engine/common/ports/HTML5.cpp
	$(TARGET)_LDFLAGS +=
endif

ifeq ($(TARGET_OS),darwin)
	$(TARGET)_SRCS +=\
		$(SDL_NET_SRCS) \
		engine/common/ports/Unix.cpp \
		engine/common/ports/Darwin.cpp \
		engine/common/ports/CocoaLog.mm \
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(NETWORKING),1)
	TMP := $(filter-out engine/common/network/Network.cpp,$($(TARGET)_SRCS))
	$(TARGET)_SRCS = $(TMP)
endif

ifneq ($(APPNAME),$(TARGET))
	$(TARGET)_IGNORE := yes
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS) -fno-rtti -fno-exceptions
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
