EMSCRIPTEN_TARGET_ROOT ?= $(HOME)

emscripten-setup:
	contrib/scripts/emscripten.sh $(EMSCRIPTEN_TARGET_ROOT)

html5-build-all:
	./configure --app=caveexpress --target-os=html5 --enable-release && $(MAKE) clean && $(MAKE)
	./configure --app=cavepacker --target-os=html5 --enable-release && $(MAKE) clean && $(MAKE)
