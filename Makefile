# Allow to override settings
CONFIG         ?= Makefile.local
-include $(CONFIG)

Q              ?= @
UPDATEDIR      := /tmp
BUILDTYPE      ?= Debug
BUILDDIR       ?= ./build/$(BUILDTYPE)
INSTALL_DIR    ?= $(BUILDDIR)
GENERATOR      := Ninja
CMAKE          ?= cmake
CMAKE_OPTIONS  ?= -DFORCE_USE_SYSTEM_LIBS=1 -DCMAKE_BUILD_TYPE=$(BUILDTYPE) -G$(GENERATOR) --graphviz=$(BUILDDIR)/deps.dot

# Android (SDK/NDK versions match .github/workflows/main.yml)
ANDROID_SCRIPT    := $(CURDIR)/contrib/scripts/android.sh
ANDROID_BUILDDIR  ?= $(CURDIR)/cp-build-android
ANDROID_SDK_ROOT  ?= $(CURDIR)/build/android_sdk
export ANDROID_SDK_ROOT

# Optional local overrides; empty recipe so the catch-all `%` does not try to build it.
$(CONFIG): ;

.PHONY: android android-setup android-apk android-emulator android-run android-install android-start android-stop android-clean android-help
.PHONY: android-caveexpress-apk android-caveexpress-install android-caveexpress-start android-caveexpress-backtrace
.PHONY: android-cavepacker-apk android-cavepacker-install android-cavepacker-start android-cavepacker-backtrace

# Native debug APKs (arm64-v8a). Same entry point as CI (`make android BUILDTYPE=Release`).
android android-apk:
	$(Q)BUILDTYPE=$(BUILDTYPE) $(ANDROID_SCRIPT) build

# SDK, NDK r28c, emulator, system image, AVD — skips packages/files that already exist.
android-setup:
	$(Q)$(ANDROID_SCRIPT) setup

android-emulator:
	$(Q)$(ANDROID_SCRIPT) emulator

android-install:
	$(Q)$(ANDROID_SCRIPT) install

android-start:
	$(Q)$(ANDROID_SCRIPT) start

# Host-ABI APK, start emulator if needed, install and launch CaveExpress (or ANDROID_GAME=cavepacker).
android-run:
	$(Q)BUILDTYPE=$(BUILDTYPE) $(ANDROID_SCRIPT) run

android-stop:
	$(Q)$(ANDROID_SCRIPT) stop

android-clean:
	$(Q)rm -rf $(ANDROID_BUILDDIR) android-project/app/build android-project/build

android-help:
	$(Q)$(ANDROID_SCRIPT) help

# CMake helper targets in the Android build tree (not the desktop BUILDDIR).
android-caveexpress-apk android-caveexpress-install android-caveexpress-start android-caveexpress-backtrace \
android-cavepacker-apk android-cavepacker-install android-cavepacker-start android-cavepacker-backtrace:
	$(Q)BUILDTYPE=$(BUILDTYPE) $(ANDROID_SCRIPT) cmake-target $@

all:
	$(Q)if [ ! -f $(BUILDDIR)/CMakeCache.txt ]; then $(CMAKE) -H$(CURDIR) -B$(BUILDDIR) $(CMAKE_OPTIONS); fi
	$(Q)$(CMAKE) --build $(BUILDDIR) --target $@
	$(Q)$(CMAKE) -E create_symlink build/Debug/compile_commands.json compile_commands.json

release:
	$(Q)$(MAKE) BUILDTYPE=Release

clean:
	$(Q)rm -rf $(BUILDDIR)

distclean:
	$(Q)git clean -fdx

ccmake:
	$(Q)ccmake -B$(BUILDDIR) -S.

release-%:
	$(Q)$(MAKE) BUILDTYPE=Release $(subst release-,,$@)

%:
	$(Q)if [ ! -f $(BUILDDIR)/CMakeCache.txt ]; then $(CMAKE) -H$(CURDIR) -B$(BUILDDIR) $(CMAKE_OPTIONS); fi
	$(Q)$(CMAKE) --build $(BUILDDIR) --target $@
	$(Q)$(CMAKE) --install $(BUILDDIR) --component $@ --prefix $(INSTALL_DIR)/install-$@
	$(Q)$(CMAKE) -E create_symlink $(BUILDDIR)/compile_commands.json compile_commands.json

dependency-%:
	$(Q)$(CMAKE) -H$(CURDIR) -B$(BUILDDIR) $(CMAKE_OPTIONS)
	$(Q)dot -Tsvg $(BUILDDIR)/deps.dot.$(subst dependency-,,$@) -o $(BUILDDIR)/deps.dot.$(subst dependency-,,$@).svg;
	$(Q)xdg-open $(BUILDDIR)/deps.dot.$(subst dependency-,,$@).svg;

caveexpress-pngquant:
	$(Q)pngquant -f --ext .png contrib/assets/png/caveexpress/*.png

caveexpress-textures: textureatlas
	./textureatlas contrib/assets/png/caveexpress*.tps
