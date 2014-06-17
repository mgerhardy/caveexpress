MAKEFILEPATH=$(CURDIR)/$(lastword $(MAKEFILE_LIST))
NACL_VERSION=35
NACL_SDK_ROOT=$(realpath $(dir $(MAKEFILEPATH))/../nacl_sdk/pepper_$(NACL_VERSION))

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
