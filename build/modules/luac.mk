TARGET             := luac

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $(LUA_LIBS)
$(TARGET)_CFLAGS   += $(LUA_CFLAGS)
$(TARGET)_SRCS      = \
	$(LUA_SRCS) \
	libs/lua/luac.cpp

ifneq ($(findstring $(TARGET_OS), mingw32 mingw64 mingw64_64),)
	$(TARGET)_SRCS += common/ports/project.rc
endif

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS)
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)

ifeq ($(HAVE_LUA_H),1)
	$(TARGET)_IGNORE := yes
endif

$(TARGET)_IGNORE   := yes
