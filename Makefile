CONFIG ?= Makefile.local
-include $(CONFIG)

ifneq ($(ADDITIONAL_PATH),)
PATH        := $(ADDITIONAL_PATH):$(PATH)
endif

HOST_OS     ?= $(shell uname | sed -e s/_.*// | tr '[:upper:]' '[:lower:]')
TARGET_OS   ?= $(HOST_OS)
TARGET_ARCH ?= $(shell uname -m | sed -e s/i.86/i386/)
ARCH        ?= $(shell uname -m)
CONFIG_H    ?= $(TARGET_OS)

ifneq ($(findstring $(HOST_OS),sunos darwin),)
  TARGET_ARCH ?= $(shell uname -p | sed -e s/i.86/i386/)
endif

MODE        ?= debug
PREFIX      ?= /usr/local
WINDRES     ?= windres
CC          ?= gcc
CXX         ?= g++
Q           ?= @
BUILDDIR    ?= $(MODE)-$(APPNAME)-$(TARGET_OS)-$(TARGET_ARCH)
PROJECTSDIR ?= build/projects
PROJECTS    := $(sort $(patsubst $(PROJECTSDIR)/%.mk,%,$(wildcard $(PROJECTSDIR)/*.mk)))
SRCDIR      := src
BASEROOT    := base
BASEDIR     := $(BASEROOT)/$(APPNAME)
TARGETS     := $(sort $(patsubst build/modules/%.mk,%,$(wildcard build/modules/*.mk)))
DISABLED    := $(filter-out $(TARGETS),$(TARGETS_TMP))
STEAM_ROOT  ?= /home/$(USER)/Downloads/steam-runtime-sdk_2013-09-05

ifeq ($(DISABLE_DEPENDENCY_TRACKING),)
  DEP_FLAGS := -MP -MD -MT $$@
endif
CONFIGURE_PREFIX ?=

CONFIGURE_OPTIONS += --prefix=$(PREFIX) --target-os=$(TARGET_OS) --app=$(APPNAME)
ifeq ($(MODE),release)
CONFIGURE_OPTIONS += --enable-release
endif
ifeq ($(USE_CCACHE),1)
CONFIGURE_OPTIONS += --enable-ccache
endif
ifeq ($(HD_VERSION),1)
CONFIGURE_OPTIONS += --enable-hd
endif

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 755 -s
INSTALL_SCRIPT  ?= $(INSTALL) -m 755
INSTALL_DIR     ?= $(INSTALL) -d
INSTALL_MAN     ?= $(INSTALL) -m 444
INSTALL_DATA    ?= $(INSTALL) -m 444

ifeq ($(USE_CCACHE),1)
CCACHE := ccache
else
CCACHE :=
endif

.PHONY: all
ifneq ($(findstring $(TARGET_OS), android ouya),)
all: Makefile.local android
else
ifeq ($(TARGET_OS),html5)
ifeq ($(EMSCRIPTEN_CALLED),1)
all: Makefile.local data $(TARGETS)
else
all: Makefile.local emscripten
endif
else
all: Makefile.local data $(TARGETS)
endif
endif

.PHONY: data
data: textures lang filelist $(SRCDIR)/engine/common/ports/project.rc

Makefile.local: configure
	$(Q)$(CONFIGURE_PREFIX) ./configure $(CONFIGURE_OPTIONS)
	$(Q)$(MAKE)

include build/flags.mk
include build/modes/$(MODE).mk
include build/default.mk
include build/install.mk
include build/platforms/$(TARGET_OS).mk
include build/nacl.mk
include build/emscripten.mk
include build/tools.mk

# TODO: libs should go into the same dir to build them only once
ASSEMBLE_OBJECTS = $(patsubst %, $(BUILDDIR)/$(1)/%.o, $(filter %.c %.cc %.cpp %.rc %.m %.mm, $($(1)_SRCS)))

define INCLUDE_PROJECT_RULE
include $(PROJECTSDIR)/$(1).mk
endef

define INCLUDE_RULE
include build/modules/$(1).mk
ifndef $(1)_DISABLE
ifndef $(1)_IGNORE
$(1)_ACTIVE := 1
endif
endif
endef
$(foreach TARGET,$(TARGETS),$(eval $(call INCLUDE_RULE,$(TARGET))))

.PHONY: clean
clean: $(addprefix clean-,$(TARGETS))
	$(Q)rm -rf $(BUILDDIR)

.PHONY: distclean
distclean: clean
	$(Q)rm -rf $(ANDROID_PROJECT)/assets
	$(Q)rm -rf $(ANDROID_PROJECT)/bin
	$(Q)rm -rf $(ANDROID_PROJECT)/gen
	$(Q)rm -rf $(ANDROID_PROJECT)/libs
	$(Q)rm -rf $(ANDROID_PROJECT)/obj
	$(Q)rm -rf $(ANDROID_PROJECT)/local.properties

.PHONY: install
install: install-pre $(addprefix install-,$(TARGETS))

.PHONY: strip
strip: $(addprefix strip-,$(TARGETS))

$(SRCDIR)/engine/common/ports/project.rc: $(SRCDIR)/engine/common/ports/project.rc.in
	@echo '===> RC [$@]'
	$(Q)sed "s/%%APPNAME%%/$(APPNAME)/" $< > $@
	$(Q)sed -i "s/%%APPNAME_FULL%%/$(APPNAME_FULL)/" $@
	$(Q)sed -i "s/%%VERSION%%/$(VERSION)/" $@

$(CONFIG_H)-config.h: configure
	@echo "restarting configure for $(CONFIG_H)"
	$(Q)$(CONFIGURE_PREFIX) ./configure $(CONFIGURE_OPTIONS)
	$(Q)$(MAKE)

define BUILD_RULE
ifeq ($($(1)_ACTIVE),1)
-include $($(1)_OBJS:.o=.d)

# if the target filename differs:
ifneq ($(1),$($(1)_FILE))
.PHONY: $(1)
$(1): $(TARGET_OS)-config.h $($(1)_FILE)
endif

$($(1)_FILE): build/modules/$(1).mk $(foreach DEP,$($(1)_DEPS),$($(DEP)_FILE)) $($(1)_OBJS)
	@echo '===> LD [$$@]'
	$(Q)mkdir -p $$(dir $($(1)_FILE))
	$(Q)$(CROSS)$($(1)_LINKER) $($(1)_OBJS) $(LDFLAGS) $($(1)_LDFLAGS) -o $($(1)_FILE)

$(BUILDDIR)/$(1)/%.c.o: $(SRCDIR)/%.c
	@echo '===> CC [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CCACHE) $(CROSS)$(CC) $(CCFLAGS) $($(1)_CCFLAGS) -c -o $$@ $$< $(DEP_FLAGS)

$(BUILDDIR)/$(1)/%.m.o: $(SRCDIR)/%.m
	@echo '===> CC [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CCACHE) $(CROSS)$(CC) $(CCFLAGS) $($(1)_CCFLAGS) -c -o $$@ $$< $(DEP_FLAGS)

$(BUILDDIR)/$(1)/%.mm.o: $(SRCDIR)/%.mm
	@echo '===> CC [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CCACHE) $(CROSS)$(CC) $(CCFLAGS) $($(1)_CCFLAGS) -c -o $$@ $$< $(DEP_FLAGS)

$(BUILDDIR)/$(1)/%.rc.o: $(SRCDIR)/%.rc
	@echo '===> WINDRES [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CROSS)$(WINDRES) -D FULL_PATH_RC_FILE -i $$< -o $$@

$(BUILDDIR)/$(1)/%.cc.o: $(SRCDIR)/%.cc
	@echo '===> CXX [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CCACHE) $(CROSS)$(CXX) $(CXXFLAGS) $($(1)_CXXFLAGS) -c -o $$@ $$< $(DEP_FLAGS)

$(BUILDDIR)/$(1)/%.cpp.o: $(SRCDIR)/%.cpp
	@echo '===> CXX [$(1)] $$<'
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CCACHE) $(CROSS)$(CXX) $(CXXFLAGS) $($(1)_CXXFLAGS) -c -o $$@ $$< $(DEP_FLAGS)

.PHONY: clean-$(1)
clean-$(1):
	@echo 'Cleaning up $(1)'
	$(Q)rm -rf $(BUILDDIR)/$(1) $($(1)_FILE)

.PHONY: strip-$(1)
strip-$(1): $($(1)_FILE)
	@echo 'Stripping $$<'
	$(Q)$(CROSS)strip $$<

install-$(1): $($(1)_FILE)
	@echo 'Install $$<'
	$(Q)$(INSTALL_DIR) $(PKGDATADIR)/$(dir $($(1)_FILE))
	$(Q)$(INSTALL_PROGRAM) $$< $(PKGDATADIR)/$$<

else
# if this target is ignored, just do nothing
$(1):
	@echo "===> NOP [$$@] is disabled"
clean-$(1):
	@echo "module is disabled"
strip-$(1):
	@echo "module is disabled"
install-$(1):
	@echo "module is disabled"
endif
endef
$(foreach TARGET,$(TARGETS),$(eval $(call BUILD_RULE,$(TARGET))))

$(foreach PROJECT,$(PROJECTS),$(eval $(call INCLUDE_PROJECT_RULE,$(PROJECT))))

.PHONY: run-configure
run-configure:
	$(Q)$(CONFIGURE_PREFIX) ./configure $(CONFIGURE_OPTIONS)

include build/android.mk

CONFIGURE_RELEASE_FLAGS := --enable-ccache --with-embedded-sqlite3 --with-embedded-tinyxml2 --with-embedded-SDL2_mixer --with-embedded-SDL2_image --with-embedded-SDL2_net --with-embedded-sdl2 --enable-release --enable-only-$(APPNAME)

release:
	./configure --target-os=mingw64 --enable-ccache --with-embedded-sqlite3 --with-embedded-tinyxml2 --enable-release --enable-only-$(APPNAME); \
	$(MAKE) Q= clean; \
	$(MAKE) Q=; \
	$(MAKE) Q= archives; \

release32:
	echo "Make sure to execute shell-i386.sh from the steam sdk"
	./configure --target-arch=i386 --target-os=linux --enable-only-$(APPNAME) $(CONFIGURE_RELEASE_FLAGS); \
	$(MAKE) Q= clean; \
	$(MAKE) Q= STEAM_RUNTIME_TARGET_ARCH=i386 STEAM_RUNTIME_HOST_ARCH=$(HOST_ARCH) STEAM_RUNTIME_ROOT=$(STEAM_ROOT)/runtime/i386 PATH=$(STEAM_ROOT)/bin:$(PATH); \
	$(MAKE) Q= archives; \

release64:
	echo "Make sure to execute shell-amd64.sh from the steam sdk"
	./configure --target-arch=x86_64 --target-os=linux --enable-only-$(APPNAME) $(CONFIGURE_RELEASE_FLAGS); \
	$(MAKE) Q= clean; \
	$(MAKE) Q= STEAM_RUNTIME_TARGET_ARCH=amd64 STEAM_RUNTIME_HOST_ARCH=$(HOST_ARCH) STEAM_RUNTIME_ROOT=$(STEAM_ROOT)/runtime/amd64 PATH=$(STEAM_ROOT)/bin:$(PATH); \
	$(MAKE) Q= archives; \

ifeq ($(TARGET_OS),html5)
.PHONY: emscripten
emscripten: data
	$(Q)EMCC_FAST_COMPILER=1 $(EMSCRIPTEN_ROOT)/emmake $(MAKE) EMSCRIPTEN_CALLED=1
endif

BASEDIRS := $(patsubst %/,%,$(patsubst $(BASEDIR)/%,%,$(sort $(dir $(wildcard $(BASEDIR)/*/)))))
filelist: $(SRCDIR)/dir.h

$(SRCDIR)/dir.h:
	@echo "==> Create filelist for base directories"
	$(Q)contrib/scripts/filelist.sh "$@" "$(BAESDIR)" $(BASEDIRS)

lang:
	$(Q)for i in en_GB de; do contrib/scripts/lang.sh "$$i" "$(APPNAME)"; done

.PHONY: ios
ios:
	$(Q)xcodebuild -sdk iphoneos6.1 -project ios/CaveExpress.xcodeproj
