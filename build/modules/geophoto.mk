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
	$(wildcard $(SRCDIR)/geophoto/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/common/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/sprites/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/textures/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/sound/sdl/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/sound/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/round/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/round//persister/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/ui/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/ui/nodes/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/client/ui/windows/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/shared/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/shared/network/*.cpp) \
	$(wildcard $(SRCDIR)/geophoto/server/*.cpp) ) \
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
