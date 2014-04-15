CONFIG ?= Makefile.local
-include $(CONFIG)

ifneq ($(ADDITIONAL_PATH),)
PATH        := $(ADDITIONAL_PATH):$(PATH)
endif

HOST_OS     ?= $(shell uname | sed -e s/_.*// | tr '[:upper:]' '[:lower:]')
TARGET_OS   ?= $(HOST_OS)
TARGET_ARCH ?= $(shell uname -m | sed -e s/i.86/i386/)
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
BUILDDIR    ?= $(MODE)-$(TARGET_OS)-$(TARGET_ARCH)
PROJECTSDIR ?= build/projects
PROJECTS    := $(sort $(patsubst $(PROJECTSDIR)/%.mk,%,$(wildcard $(PROJECTSDIR)/*.mk)))
SRCDIR      := src
BASEROOT    := base
BASEDIR     := $(BASEROOT)/$(APPNAME)
TARGETS_TMP := $(sort $(patsubst build/modules/%.mk,%,$(wildcard build/modules/*.mk)))
TARGETS     := $(filter-out $(foreach TARGET,$(TARGETS_TMP),$(if $($(TARGET)_DISABLE),$(TARGET),)),$(TARGETS_TMP))
DISABLED    := $(filter-out $(TARGETS),$(TARGETS_TMP))
STEAM_ROOT  ?= /home/mattn/Downloads/steam-runtime-sdk_2013-09-05

ifeq ($(DISABLE_DEPENDENCY_TRACKING),)
  DEP_FLAGS := -MP -MD -MT $$@
endif
CONFIGURE_PREFIX ?=

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
data: textures lang

Makefile.local: configure
	./configure --target-os=$(TARGET_OS)

include build/flags.mk
include build/modes/$(MODE).mk
include build/default.mk
include build/platforms/$(TARGET_OS).mk
include build/tools.mk
include build/install.mk

# TODO: libs should go into the same dir to build them only once
ASSEMBLE_OBJECTS = $(patsubst %, $(BUILDDIR)/$(1)/%.o, $(filter %.c %.cc %.cpp %.rc %.m %.mm, $($(1)_SRCS)))

define INCLUDE_PROJECT_RULE
include $(PROJECTSDIR)/$(1).mk
endef

define INCLUDE_RULE
include build/modules/$(1).mk
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

$(CONFIG_H)-config.h: configure
	@echo "restarting configure for $(CONFIG_H)"
	$(Q)$(CONFIGURE_PREFIX) ./configure $(CONFIGURE_OPTIONS) --target-os=$(TARGET_OS)
	$(Q)$(MAKE)

define BUILD_RULE
ifndef $(1)_DISABLE
ifndef $(1)_IGNORE

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
clean-$(1):
	@echo "module is disabled"
strip-$(1):
	@echo "module is disabled"
install-$(1):
	@echo "module is disabled"
endif
endif
endef
$(foreach TARGET,$(TARGETS),$(eval $(call BUILD_RULE,$(TARGET))))

$(foreach PROJECT,$(PROJECTS),$(eval $(call INCLUDE_PROJECT_RULE,$(PROJECT))))

.PHONY: run-configure
run-configure:
	$(Q)$(CONFIGURE_PREFIX) ./configure $(CONFIGURE_OPTIONS)

ANDROID_PROJECT=android-project
ifneq ($(findstring $(TARGET_OS), android ouya),)
ifeq ($(DEBUG),)
ANT_TARGET          := release
ANT_INSTALL_TARGET  := installr
MANIFEST_DEBUGGABLE := true
APPLICATION_MK      := APP_OPTIM := debug
else
ANT_TARGET          := debug
ANT_INSTALL_TARGET  := installd
MANIFEST_DEBUGGABLE := false
APPLICATION_MK      :=
endif
ifeq ($(Q),)
NDK_VERBOSE:="V=1"
else
NDK_VERBOSE:=
endif

ifeq ($(TARGET_OS),android)
PERMISSIONS := <uses-permission android:name="com.android.vending.BILLING" />
META_DATA = <meta-data android:value="true" android:name="ADMOB_ALLOW_LOCATION_FOR_ADS" />
#META_DATA += <meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/app_id" />
#META_DATA += <meta-data android:name="com.google.android.gms.appstate.APP_ID" android:value="@string/app_id" />
META_DATA += <meta-data android:name="com.google.android.gms.version" android:value="@integer/google_play_services_version" />
ANDROID_REFERENCED_LIBS := android.library.reference.1=google-play-services_lib
else
PERMISSIONS :=
META_DATA :=
ANDROID_REFERENCED_LIBS :=
endif

ICON := $(APPNAME)-icon.png

.PHONY: clean-android-libs
clean-android-libs:
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/.project
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/assets/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/bin/
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/build.xml
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/gen/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/libs/
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/local.properties
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/proguard-project.txt
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/res/drawable-ldpi/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/res/layout/

.PHONY: clean-android
clean-android: clean-android-libs
	@echo "===> ANDROID [clean project]"
	$(Q)rm -rf $(ANDROID_PROJECT)/assets
	$(Q)rm -rf $(ANDROID_PROJECT)/bin
	$(Q)rm -rf $(ANDROID_PROJECT)/obj
	$(Q)rm -rf $(ANDROID_PROJECT)/gen
	$(Q)rm -f $(ANDROID_PROJECT)/local.properties
	$(Q)rm -f $(ANDROID_PROJECT)/default.properties
	$(Q)rm -f $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)rm -f $(ANDROID_PROJECT)/build.xml
	$(Q)rm -f $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)rm -f $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)rm -f $(SRCDIR)/Android.mk

android-update-project: $(ANDROID_PROJECT)/local.properties $(ANDROID_PROJECT)/build.xml $(ANDROID_PROJECT)/AndroidManifest.xml

$(ANDROID_PROJECT)/AndroidManifest.xml: $(SRCDIR)/Android.mk.in $(ANDROID_PROJECT)/AndroidManifest.xml.in $(ANDROID_PROJECT)/jni/Application.mk.in $(ANDROID_PROJECT)/strings.xml.in $(CONFIG_H)-config.h
	$(Q)cp $(ANDROID_PROJECT)/AndroidManifest.xml.in $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@DEBUGGABLE@/$(MANIFEST_DEBUGGABLE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's,@PERMISSIONS@,$(PERMISSIONS),g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's,@META_DATA@,$(META_DATA),g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@VERSION@/$(VERSION)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@VERSION_CODE@/$(VERSION_CODE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@JAVA_PACKAGE@/$(JAVA_PACKAGE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@MIN_ANDROID_SDK@/$(MIN_ANDROID_SDK)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)cp $(ANDROID_PROJECT)/strings.xml.in $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)cp $(ANDROID_PROJECT)/jni/Application.mk.in $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)sed -i 's/@APPLICATION_MK@/$(APPLICATION_MK)/g' $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)cp $(SRCDIR)/Android.mk.in $(SRCDIR)/Android.mk
	$(Q)sed -i 's/@APPNAME@/$(APPNAME)/g' $(SRCDIR)/Android.mk
	$(Q)sed -i 's/import org.caveexpress.*R;/import $(JAVA_PACKAGE).R;/g' $(ANDROID_PROJECT)/src/org/base/game/GameHelper.java

android-update-sdk-version:
	$(Q)cp $(ANDROID_PROJECT)/default.properties.in $(ANDROID_PROJECT)/default.properties
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/default.properties
	$(Q)cp $(ANDROID_PROJECT)/project.properties.in $(ANDROID_PROJECT)/project.properties
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/project.properties
	$(Q)sed -i 's/@ANDROID_REFERENCED_LIBS@/$(ANDROID_REFERENCED_LIBS)/g' $(ANDROID_PROJECT)/project.properties

$(ANDROID_PROJECT)/build.xml: $(ANDROID_PROJECT)/build.xml.in $(CONFIG_H)-config.h
	$(Q)cp $(ANDROID_PROJECT)/build.xml.in $(ANDROID_PROJECT)/build.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/build.xml
	$(Q)sed -i 's:@EXCLUDE_PACKAGE_PATHS@:$(EXCLUDE_JAVA_PACKAGE_PATHS):g' $(ANDROID_PROJECT)/build.xml

$(ANDROID_PROJECT)/local.properties: $(ANDROID_PROJECT)/AndroidManifest.xml $(ANDROID_PROJECT)/build.xml
	@echo "===> ANDROID [update project]"
	$(Q)cd $(ANDROID_PROJECT); SDK=`android list sdk | grep "SDK Platform Android 3.2" | awk -F'-' ' { print $$1 }'`; [ -n "$$SDK" ] && android update sdk -u -s -t $$SDK || echo
	$(Q)cd $(ANDROID_PROJECT); SDK=`android list sdk | grep "SDK Platform Android 4.1.2" | awk -F'-' ' { print $$1 }'`; [ -n "$$SDK" ] && android update sdk -u -s -t $$SDK || echo
	#$(Q)cd $(ANDROID_PROJECT); SDK=`android list sdk --all | grep "Build-tools, revision 18.1.1" | awk -F'-' ' { print $$1 }'`; [ -n "$$SDK" ] && android update sdk -a -u -s -t build-tools-18.1.1 || echo
	$(Q)cd $(ANDROID_PROJECT) && android update project -p . -t android-13
	$(Q)cd $(ANDROID_PROJECT) && android update lib-project -t android-13 --path google-play-services_lib

.PHONY: android-copy-assets
android-copy-assets: data
	@echo "===> CP [copy assets]"
	$(Q)rm -rf $(ANDROID_PROJECT)/assets
	$(Q)rm -rf $(ANDROID_PROJECT)/libs
	$(Q)mkdir -p $(ANDROID_PROJECT)/libs
	$(Q)mkdir -p $(ANDROID_PROJECT)/assets/$(BASEROOT)
	$(Q)cp -rf $(BASEDIR) $(ANDROID_PROJECT)/assets/$(BASEROOT)
	$(Q)rm -f $(ANDROID_PROJECT)/assets/$(BASEDIR)/maps/test*
	$(Q)rm -f $(ANDROID_PROJECT)/assets/$(BASEDIR)/maps/empty*
	$(Q)for i in hdpi ldpi mdpi xhdpi; do mkdir -p $(ANDROID_PROJECT)/res/drawable-$${i}; cp -f contrib/$(ICON) $(ANDROID_PROJECT)/res/drawable-$${i}/icon.png; done
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then cp contrib/installer/ouya/ouya_icon.png $(ANDROID_PROJECT)/res/drawable-xhdpi; fi
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then mkdir -p $(ANDROID_PROJECT)/res/raw && cp ~/ouyakey.der $(ANDROID_PROJECT)/res/raw/key.der; fi
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then cp contrib/installer/ouya/*.jar $(ANDROID_PROJECT)/libs; fi
	$(Q)if [ "$(TARGET_OS)" = "android" ]; then mkdir -p $(ANDROID_PROJECT)/libs && cp contrib/installer/android/*.jar $(ANDROID_PROJECT)/libs; fi

.PHONY: android-build
android-build: android-update-sdk-version
	@echo "===> NDK [native build]"
	$(Q)cd $(ANDROID_PROJECT) && NDK_CCACHE=ccache ndk-build -j 4 $(NDK_VERBOSE)
	@echo "===> ANT [java build]"
	$(Q)cd $(ANDROID_PROJECT) && ant $(ANT_TARGET)

.PHONY: android-install
android-install:
	$(Q)cd $(ANDROID_PROJECT) && ant $(ANT_INSTALL_TARGET)

android-emulator-create:
	android create avd -n CaveExpress -t android-13

android-emulator-start:
	emulator -avd CaveExpress
# -scale 96dpi -dpi-device 160

android-emulator-run: android-emulator-start android-install android-run

.PHONY: android-all
.NOTPARALLEL:
android-all: clean-android-libs android-update-project android-copy-assets android-build android-uninstall android-install

.PHONY: android
.NOTPARALLEL:
android: clean-android-libs android-update-project android-copy-assets android-build

android-backtrace:
	adb logcat | ndk-stack -sym $(ANDROID_PROJECT)/obj/local/armeabi

android-run:
	adb shell am start -n $(JAVA_PACKAGE)/$(JAVA_PACKAGE).$(APPNAME_FULL)

android-uninstall:
	$(Q)cd $(ANDROID_PROJECT) && ant uninstall
endif

release-android:
	./configure --enable-ccache --target-os=android --enable-release && $(MAKE) clean-android clean && $(MAKE)
	cp $(ANDROID_PROJECT)/bin/*-release.apk $(APPNAME_FULL)-release.apk
	./configure --enable-ccache --target-os=ouya --enable-release && $(MAKE) clean-android clean && $(MAKE)
	cp $(ANDROID_PROJECT)/bin/*-release.apk $(APPNAME_FULL)-ouya-release.apk

CONFIGURE_RELEASE_FLAGS := --enable-ccache --with-embedded-sqlite3 --with-embedded-tinyxml2 --with-embedded-SDL2_mixer --with-embedded-SDL2_image --with-embedded-SDL2_net --with-embedded-sdl2 --enable-release --enable-only-$(APPNAME)

release:
	./configure --target-os=mingw64 --enable-ccache --with-embedded-sqlite3 --with-embedded-tinyxml2 --enable-release --enable-only-$(APPNAME); \
	$(MAKE) Q= clean; \
	$(MAKE) Q=; \
	$(MAKE) Q= archives; \

release32:
	echo "Make sure to execute shell-i386.sh from the steam sdk"
	./configure --target-arch=i386 --target-os=linux $(CONFIGURE_RELEASE_FLAGS); \
	$(MAKE) Q= clean; \
	$(MAKE) Q= STEAM_RUNTIME_TARGET_ARCH=i386 STEAM_RUNTIME_HOST_ARCH=$(HOST_ARCH) STEAM_RUNTIME_ROOT=$(STEAM_ROOT)/runtime/i386 PATH=$(STEAM_ROOT)/bin:$(PATH); \
	$(MAKE) Q= archives; \

release64:
	echo "Make sure to execute shell-amd64.sh from the steam sdk"
	./configure --target-arch=x86_64 --target-os=linux $(CONFIGURE_RELEASE_FLAGS); \
	$(MAKE) Q= clean; \
	$(MAKE) Q= STEAM_RUNTIME_TARGET_ARCH=amd64 STEAM_RUNTIME_HOST_ARCH=$(HOST_ARCH) STEAM_RUNTIME_ROOT=$(STEAM_ROOT)/runtime/amd64 PATH=$(STEAM_ROOT)/bin:$(PATH); \
	$(MAKE) Q= archives; \

ifeq ($(TARGET_OS),html5)
.PHONY: emscripten
emscripten: data
	$(Q)EMCC_FAST_COMPILER=1 $(EMSCRIPTEN_ROOT)/emmake $(MAKE) EMSCRIPTEN_CALLED=1
endif

lang:
	$(Q)for i in en_GB de; do contrib/scripts/lang.sh "$$i" "$(APPNAME)"; done

.PHONY: ios
ios:
	$(Q)xcodebuild -sdk iphoneos6.1 -project ios/CaveExpress.xcodeproj
