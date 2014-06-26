TARGET             := jsonconvert

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(SDL_LIBS) $(YAJL_LIBS) $(SO_LIBS) $(LUA_LIBS) $(SQLITE3_LIBS) $(TINYXML2_LIBS) $(SDL_NET_LIBS) $(SDL_RWHTTP_LIBS) $(NCURSES_LIBS)
$(TARGET)_CFLAGS   += $(SDL_CFLAGS) $(YAJL_CFLAGS) $(TINYXML2_CFLAGS) $(SQLITE3_CFLAGS) $(LUA_CFLAGS) $(SDL_NET_CFLAGS) $(SDL_RWHTTP_CFLAGS) $(NCURSES_CFLAGS)
$(TARGET)_SRCS      = $(subst $(SRCDIR)/,, \
	$(wildcard $(SRCDIR)/caveexpress/tools/bot/*.cpp) \
	\
	$(wildcard $(SRCDIR)/caveexpress/shared/*.cpp) \
	\
	$(wildcard $(SRCDIR)/caveexpress/shared/network/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/common/campaign/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/common/campaign/persister/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/common/network/*.cpp) \
	\
	$(wildcard $(SRCDIR)/engine/common/*.cpp) \
	) \
	\
	$(SDL_SRCS) \
	\
	$(SDL_NET_SRCS) \
	\
	$(TINYXML2_SRCS) \
	\
	$(YAJL_SRCS) \
	\
	$(SQLITE3_SRCS) \
	\
	$(LUA_SRCS)

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

ifneq ($(NETWORKING),1)
	TMP := $(filter-out engine/common/network/Network.cpp,$($(TARGET)_SRCS))
	$(TARGET)_SRCS = $(TMP)
	$(TARGET)_IGNORE := yes
endif

ifneq ($(APPNAME),caveexpress)
	$(TARGET)_IGNORE := yes
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS)
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
