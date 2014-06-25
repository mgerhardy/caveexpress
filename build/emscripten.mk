EMSCRIPTEN_TARGET_ROOT ?= ~

emscripten-setup:
	@echo "Setup build environment for emscripten"
	$(Q)git clone https://github.com/kripken/emscripten $(EMSCRIPTEN_TARGET_ROOT)/emscripten; \
	git clone https://github.com/kripken/emscripten-fastcomp $(EMSCRIPTEN_TARGET_ROOT)//emscripten-fastcomp; \
	git clone https://github.com/kripken/emscripten-fastcomp-clang $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcom/tools/clang; \
	mkdir -p $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcom/build; cd $(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcom/build; \
	../configure --enable-optimized --disable-assertions --enable-targets=host,js; make -j 2; \
	echo "LLVM_ROOT=$(EMSCRIPTEN_TARGET_ROOT)/emscripten-fastcom/build/bin/Release" >> ~/.emscripten
