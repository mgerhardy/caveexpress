TARGET             := tests

# if the linking should be static
$(TARGET)_STATIC   ?= $(STATIC)
ifeq ($($(TARGET)_STATIC),1)
$(TARGET)_LDFLAGS  += -static
endif

$(TARGET)_LINKER   := $(CXX)
$(TARGET)_FILE     := $(TARGET)$(EXE_EXT)
$(TARGET)_LDFLAGS  += $($(APPNAME)_LDFLAGS) $(GTEST_LIBS)
$(TARGET)_CFLAGS   += $($(APPNAME)_CFLAGS) $(GTEST_CFLAGS) -DNO_DEBUG_RENDERER
$(TARGET)_SRCS      = $(filter-out Main.cpp,$(subst $(SRCDIR)/,, \
	$(wildcard $(SRCDIR)/tests/*.cpp) \
	$(wildcard $(SRCDIR)/tests/$(APPNAME)/*.cpp) \
	) \
	$($(APPNAME)_SRCS) \
	\
	$(GTEST_SRCS) \
	)

$(TARGET)_OBJS     := $(call ASSEMBLE_OBJECTS,$(TARGET))
$(TARGET)_CXXFLAGS := $($(TARGET)_CFLAGS)
$(TARGET)_CCFLAGS  := $($(TARGET)_CFLAGS)
