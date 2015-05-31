TARGET_FILE=$(APPNAME)$(EXE_EXT)
GDB_INIT=naclgdb.init

nacl-setup:
	$(Q)echo "Download sdk..."
	$(Q)wget http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip
	$(Q)unzip -o nacl_sdk.zip
	$(Q)rm -f nacl_sdk.zip
	$(Q)echo "Update sdks..."
	$(Q)cd nacl_sdk && ./naclsdk update
	$(Q)echo "Configure paths..."; \
	echo "export NACL_SDK_ROOT=$(NACL_SDK_ROOT)" >> ~/.bashrc; \
	echo "export PATH=\$$PATH:$(NACL_TOOLCHAIN_ROOT)/bin" >> ~/.bashrc;

%_x86-32.nexe: $(TARGET_FILE)
	@echo "===> Translate 32"
	$(Q)$(NACL_TOOLCHAIN_ROOT)/bin/pnacl-translate $^ --allow-llvm-bitcode-input -arch x86-32 -o $@

%_x86-64.nexe: $(TARGET_FILE)
	@echo "===> Translate 64"
	$(Q)$(NACL_TOOLCHAIN_ROOT)/bin/pnacl-translate $^ --allow-llvm-bitcode-input -arch x86-64 -o $@

nacl-translate: $(INSTALLER_DIR)/nacl/$(TARGET_FILE:.pexe=_x86-32.nexe) $(INSTALLER_DIR)/nacl/$(TARGET_FILE:.pexe=_x86-64.nexe)
	$(Q)$(NACL_SDK_ROOT)/tools/create_nmf.py -L $(NACL_TOOLCHAIN_ROOT)/lib -o $(INSTALLER_DIR)/nacl/$(APPNAME)-debug.nmf $^

nacl-start:
	$(Q)cd $(INSTALLER_DIR)/nacl/; \
	python -m SimpleHTTPServer 4242 & \
	NACL_DEBUG_ENABLE=1 PPAPI_BROWSER_DEBUG=1 $(CHROME_BIN) http://127.0.0.1:4242/$(APPNAME).html

NEXE ?= $(INSTALLER_DIR)/nacl/$(TARGET_FILE:.pexe=_$(ARCH).nexe)
# https://developer.chrome.com/native-client/devguide/devcycle/debugging
nacl-start-debug: nacl-installer nacl-translate
	@echo "===> Create html"
	$(Q)$(NACL_SDK_ROOT)/tools/create_html.py $(INSTALLER_DIR)/nacl/$(APPNAME)-debug.nmf
	#echo "nacl-manifest \"$(INSTALLER_DIR)/nacl/$(APPNAME)-debug.nmf\"" >> $(GDB_INIT)
	$(Q)echo "target remote localhost:4014" > $(GDB_INIT); \
	echo "remote get nexe \"$(NEXE)\"" >>  $(GDB_INIT); \
	echo "file \"$(NEXE)\"" >> $(GDB_INIT); \
	echo "nacl-irt \"$(CHROME_NACL_IRT)\"" >> $(GDB_INIT)
	$(Q)cd $(INSTALLER_DIR)/nacl/; \
	python -m SimpleHTTPServer 4242 & \
	NACL_DEBUG_ENABLE=1 PPAPI_BROWSER_DEBUG=1 $(CHROME_BIN) --enable-nacl --enable-nacl-debug --no-sandbox http://127.0.0.1:4242/$(APPNAME)-debug.html & \
	$(NACL_SDK_ROOT)/toolchain/$(HOST_OS)_x86_$(NACL_LIB)/bin/$(ARCH)-nacl-gdb -x $(GDB_INIT)

nacl-installer: $(INSTALLER_DIR)/nacl/$(APPNAME).html $(INSTALLER_DIR)/nacl/$(APPNAME).nmf $(INSTALLER_DIR)/nacl/$(TARGET_FILE)
	@echo "===> Copy assets"
	$(Q)rm -rf $(INSTALLER_DIR)/nacl/$(BASEROOT)
	$(Q)mkdir -p $(INSTALLER_DIR)/nacl/$(BASEROOT)
	$(Q)cp -rf $(BASEDIR) $(INSTALLER_DIR)/nacl/$(BASEROOT)

$(INSTALLER_DIR)/nacl/$(TARGET_FILE): $(TARGET_FILE)
	@echo "===> Finalize"
	$(Q)$(NACL_TOOLCHAIN_ROOT)/bin/pnacl-finalize --compress $< -o $@

$(INSTALLER_DIR)/nacl/%.nmf: $(INSTALLER_DIR)/nacl/$(TARGET_FILE)
	@echo "===> Create manifest"
	$(Q)$(NACL_SDK_ROOT)/tools/create_nmf.py -L $(NACL_TOOLCHAIN_ROOT)/lib -o $@ $<

$(INSTALLER_DIR)/nacl/%.html: $(INSTALLER_DIR)/nacl/$(APPNAME).nmf
	@echo "===> Create html"
	$(Q)$(NACL_SDK_ROOT)/tools/create_html.py $<
	$(Q)sed -i 's/x-nacl/x-pnacl/g' $@
