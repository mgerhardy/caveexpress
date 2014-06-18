TARGET             := soundmapper

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(SDL_LIBS) -lz $(SO_LIBS) $(LUA_LIBS) $(SQLITE3_LIBS)
$(TARGET)_CFLAGS   += $(SDL_CFLAGS) $(LUA_CFLAGS) $(SQLITE3_CFLAGS)
$(TARGET)_SRCS      = \
	caveexpress/tools/SoundMapper.cpp \
	caveexpress/shared/CaveExpressAnimation.cpp \
	caveexpress/shared/CaveExpressEntityType.cpp \
	\
	engine/common/URI.cpp \
	engine/common/File.cpp \
	engine/common/String.cpp \
	engine/common/ConfigVar.cpp \
	engine/common/CommandSystem.cpp \
	engine/common/Logger.cpp \
	engine/common/SQLite.cpp \
	engine/common/FileSystem.cpp \
	engine/common/LUA.cpp \
	engine/common/ConfigManager.cpp \
	engine/common/ConfigPersisterSQL.cpp \
	engine/common/ThemeType.cpp \
	\
	$(SQLITE3_SRCS) \
	\
	$(LUA_SRCS) \
	\
	$(SDL_SRCS)

ifneq ($(findstring $(TARGET_OS), mingw32 mingw64 mingw64_64),)
	$(TARGET)_SRCS +=\
		engine/common/ports/Windows.cpp \
		engine/common/ports/project.rc
	$(TARGET)_LDFLAGS +=
endif

ifneq ($(findstring $(TARGET_OS), netbsd freebsd linux),)
	$(TARGET)_SRCS +=\
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
		engine/common/ports/Unix.cpp \
		engine/common/ports/Darwin.cpp \
		engine/common/ports/CocoaLog.mm \
	$(TARGET)_LDFLAGS +=
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS)
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
