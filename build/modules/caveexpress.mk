TARGET             := caveexpress

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(SDL_LIBS) $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(ZLIB_LIBS) $(OPENGL_LIBS) $(OGG_LIBS) $(VORBIS_LIBS) $(PNG_LIBS) $(NCURSES_LIBS) $(SO_LIBS) $(LUA_LIBS) $(SQLITE3_LIBS) $(TINYXML2_LIBS) $(SDL_NET_LIBS) $(SDL_RWHTTP_LIBS)
$(TARGET)_CFLAGS   += $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) $(SDL_MIXER_CFLAGS) $(ZLIB_CFLAGS) $(OPENGL_CFLAGS) $(OGG_CFLAGS) $(VORBIS_CFLAGS) $(LUA_CFLAGS) $(PNG_CFLAGS) $(NCURSES_CFLAGS) $(SQLITE3_CFLAGS) $(TINYXML2_CFLAGS) $(SDL_NET_CFLAGS) $(SDL_RWHTTP_CFLAGS)
$(TARGET)_SRCS      = $(subst $(SRCDIR)/,, \
	$(wildcard $(SRCDIR)/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/client/shaders/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sprites/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/particles/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/textures/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sound/sdl/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/sound/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/entities/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/nodes/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/windows/*.cpp) \
	$(wildcard $(SRCDIR)/engine/client/ui/layouts/*.cpp) \
	$(wildcard $(SRCDIR)/engine/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/network/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/campaign/*.cpp) \
	$(wildcard $(SRCDIR)/engine/common/campaign/persister/*.cpp) \
	$(wildcard $(SRCDIR)/engine/server/*.cpp) \
	$(wildcard $(SRCDIR)/engine/server/box2d/*.cpp) \
	\
	$(wildcard $(SRCDIR)/caveexpress/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/particles/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/entities/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/ui/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/ui/nodes/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/ui/windows/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/client/ui/windows/intro/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/shared/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/shared/network/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/map/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/events/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/entities/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/entities/modificators/*.cpp) \
	$(wildcard $(SRCDIR)/caveexpress/server/entities/npcs/*.cpp) \
	\
	$(wildcard $(SRCDIR)/libs/micropather/*.cpp) \
	\
	$(wildcard $(SRCDIR)/libs/Box2D/Common/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Collision/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Collision/Shapes/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Dynamics/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Dynamics/Contacts/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Dynamics/Joints/*.cpp) \
	$(wildcard $(SRCDIR)/libs/Box2D/Rope/*.cpp) \
	) \
	\
	$(SQLITE3_SRCS) \
	\
	$(LUA_SRCS) \
	\
	$(TINYXML2_SRCS) \
	\
	$(SDL_SRCS) \
	\
	$(SDL_IMAGE_SRCS) \
	\
	$(SDL_MIXER_SRCS) \
	\
	$(ZLIB_SRCS)

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
		engine/common/ports/NaCl.cpp
	TMP := $(filter-out engine/common/network/Network.cpp,$($(TARGET)_SRCS))
	$(TARGET)_SRCS = $(TMP)
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), html5),)
	$(TARGET)_SRCS +=\
		engine/common/ports/Unix.cpp \
		engine/common/ports/HTML5.cpp
	TMP := $(filter-out engine/common/network/Network.cpp,$($(TARGET)_SRCS))
	$(TARGET)_SRCS = $(TMP)
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

ifeq ($(TARGET_OS),android)
	$(TARGET)_SRCS +=\
		$(SDL_NET_SRCS) \
		engine/common/ports/Android.cpp \
		engine/common/ports/Unix.cpp
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(APPNAME),$(TARGET))
	$(TARGET)_IGNORE := yes
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS) -fno-rtti -fno-exceptions
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
