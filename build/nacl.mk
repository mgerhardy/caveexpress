MAKEFILEPATH   = $(CURDIR)/$(lastword $(MAKEFILE_LIST))
NACL_VERSION  ?= 35
NACL_SDK_ROOT ?= $(realpath $(dir $(MAKEFILEPATH))/../nacl_sdk/pepper_$(NACL_VERSION))
CHROME_BIN    ?= google-chrome

nacl-setup:
	$(Q)echo "Download sdk..."
	$(Q)wget http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip
	$(Q)unzip -o nacl_sdk.zip
	$(Q)rm -f nacl_sdk.zip
	$(Q)echo "Update sdks..."
	$(Q)cd nacl_sdk && ./naclsdk update
	$(Q)echo "Configure paths..."; \
	echo "export NACL_SDK_ROOT=$(NACL_SDK_ROOT)" >> ~/.bashrc; \
	echo "export PATH=\$$PATH:\$$NACL_SDK_ROOT/toolchain/$(HOST_OS)_pnacl/bin" >> ~/.bashrc;

nacl-finalize:
	$(Q)echo "Finalize"
	$(Q)$(NACL_SDK_ROOT)/toolchain/$(HOST_OS)_pnacl/bin/pnacl-finalize --compress $(caveexpress_FILE) -o contrib/installer/nacl/$(caveexpress_FILE)

nacl-translate:
	$(Q)echo "Translate"
	$(Q)$(NACL_SDK_ROOT)/toolchain/$(HOST_OS)_pnacl/bin/pnacl-translate $(caveexpress_FILE) -o contrib/installer/nacl/caveexpress.nexe -arch x86-64

nacl-start:
	$(Q)cd contrib/installer/nacl/; \
	python -m SimpleHTTPServer 4242 & \
	NACL_DEBUG_ENABLE=1 PPAPI_BROWSER_DEBUG=1 $(CHROME_BIN) http://127.0.0.1:4242

nacl-installer: nacl-finalize
	@echo "Copy assets"
	$(Q)rm -rf contrib/installer/nacl/base
	$(Q)mkdir -p contrib/installer/nacl/base
	$(Q)cp -rf $(BASEDIR) contrib/installer/nacl/base
