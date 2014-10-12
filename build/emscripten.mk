EMSCRIPTEN_TARGET_ROOT ?= $(HOME)

emscripten-setup:
	contrib/scripts/emscripten.sh $(EMSCRIPTEN_TARGET_ROOT)
