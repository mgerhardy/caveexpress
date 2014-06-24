TARGET_FILE=caveexpress$(EXE_EXT)

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

nacl-translate: $(TARGET_FILE)
	@echo "Translate"
	$(Q)$(NACL_TOOLCHAIN_ROOT)/bin/pnacl-translate $@ -o $(INSTALLER_DIR)/nacl/caveexpress.nexe -arch x86-64

nacl-start:
	$(Q)cd $(INSTALLER_DIR)/nacl/; \
	python -m SimpleHTTPServer 4242 & \
	NACL_DEBUG_ENABLE=1 PPAPI_BROWSER_DEBUG=1 $(CHROME_BIN) http://127.0.0.1:4242/caveexpress.html

nacl-installer: $(INSTALLER_DIR)/nacl/caveexpress.html $(INSTALLER_DIR)/nacl/caveexpress.nmf $(INSTALLER_DIR)/nacl/$(TARGET_FILE)
	@echo "Copy assets"
	$(Q)rm -rf $(INSTALLER_DIR)/nacl/base
	$(Q)mkdir -p $(INSTALLER_DIR)/nacl/base
	$(Q)cp -rf $(BASEDIR) $(INSTALLER_DIR)/nacl/base

$(INSTALLER_DIR)/nacl/$(TARGET_FILE): $(TARGET_FILE)
	@echo "Finalize"
	$(Q)$(NACL_TOOLCHAIN_ROOT)/bin/pnacl-finalize --compress $< -o $@

$(INSTALLER_DIR)/nacl/%.nmf: $(INSTALLER_DIR)/nacl/$(TARGET_FILE)
	@echo "Create manifest"
	$(Q)$(NACL_SDK_ROOT)/tools/create_nmf.py -L $(NACL_TOOLCHAIN_ROOT)/lib -o $@ $<

$(INSTALLER_DIR)/nacl/%.html: $(INSTALLER_DIR)/nacl/caveexpress.nmf
	@echo "Create html"
	$(Q)$(NACL_SDK_ROOT)/tools/create_html.py $<
	$(Q)sed -i 's/x-nacl/x-pnacl/g' $@
