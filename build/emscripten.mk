EMSCRIPTEN_TARGET_ROOT ?= ~

emscripten-setup:
	@echo "Setup build environment for emscripten"
	$(Q)git clone https://github.com/kripken/emscripten $(EMSCRIPTEN_TARGET_ROOT)/emscripten; \
	git clone https://github.com/kripken/emscripten-fastcomp $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcomp; \
	git clone https://github.com/kripken/emscripten-fastcomp-clang $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcomp/tools/clang; \
	mkdir -p $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcomp/build; \
	cd $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcomp/build && ../configure --enable-optimized --disable-assertions --enable-targets=host,js; make -j 2; \
	echo "Configure paths..."; \
	echo "export LLVM_ROOT=\"$(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcom/build/bin/Release\"" >> ~/.bashrc
	echo "export EMSCRIPTEN=\"$(EMSCRIPTEN_TARGET_ROOT)/emscripten\"" >> ~/.bashrc
