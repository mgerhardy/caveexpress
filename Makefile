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
CMAKE_OPTIONS  ?= -DCMAKE_BUILD_TYPE=$(BUILDTYPE) -G$(GENERATOR) --graphviz=$(BUILDDIR)/deps.dot

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
	$(Q)$(CMAKE) -E create_symlink build/Debug/compile_commands.json compile_commands.json

dependency-%:
	$(Q)$(CMAKE) -H$(CURDIR) -B$(BUILDDIR) $(CMAKE_OPTIONS)
	$(Q)dot -Tsvg $(BUILDDIR)/deps.dot.$(subst dependency-,,$@) -o $(BUILDDIR)/deps.dot.$(subst dependency-,,$@).svg;
	$(Q)xdg-open $(BUILDDIR)/deps.dot.$(subst dependency-,,$@).svg;

define UPDATE_GIT
	$(Q)if [ ! -d $(UPDATEDIR)/$(1).sync ]; then \
		git clone --depth=1 $(2) $(UPDATEDIR)/$(1).sync; \
	else \
		cd $(UPDATEDIR)/$(1).sync && git pull --depth=1 --rebase; \
	fi;
endef

update-sdl2:
	$(call UPDATE_GIT,sdl2,https://github.com/libsdl-org/SDL.git)
	rm -rf contrib/libs/sdl2/src/* contrib/libs/sdl2/include/* contrib/libs/sdl2/cmake/*
	cp -r $(UPDATEDIR)/sdl2.sync/src/* contrib/libs/sdl2/src
	cp -r $(UPDATEDIR)/sdl2.sync/wayland-protocols/* contrib/libs/sdl2/wayland-protocols
	cp -r $(UPDATEDIR)/sdl2.sync/include/* contrib/libs/sdl2/include
	cp -r $(UPDATEDIR)/sdl2.sync/cmake/* contrib/libs/sdl2/cmake

update-sdl2mixer:
	$(call UPDATE_HG,sdl2_mixer,https://hg.libsdl.org/SDL_mixer)
	rm -rf contrib/libs/sdl2_mixer/*
	cp -r $(UPDATEDIR)/sdl2_mixer.sync/src/* contrib/libs/sdl2_mixer
	cp -r $(UPDATEDIR)/sdl2_mixer.sync/include/* contrib/libs/sdl2_mixer
	cp -r $(UPDATEDIR)/sdl2_mixer.sync/external/libogg* contrib/libs/sdl2_mixer
	cp -r $(UPDATEDIR)/sdl2_mixer.sync/external/libvorbis* contrib/libs/sdl2_mixer
	git checkout -f contrib/libs/sdl2_mixer/CMakeLists.txt
