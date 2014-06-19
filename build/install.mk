ROOTDIR=$(shell pwd)
INSTALLER_DIR=$(ROOTDIR)/contrib/installer

wininstaller: strip
	$(Q)makensis -DPRODUCT_NAME=$(APPNAME) -DPRODUCT_VERSION=$(VERSION) $(INSTALLER_DIR)/windows/setup.nsi
	$(Q)md5sum $(INSTALLER_DIR)/windows/$(APPNAME)-$(VERSION).exe > $(INSTALLER_DIR)/windows/$(APPNAME)-$(VERSION).md5

archives: strip
	@echo "Create tar archive of $(APPNAME)"
	$(Q)git archive --format tar -o $(APPNAME)-$(MODE)-$(TARGET_OS)-$(TARGET_ARCH).tar.gz HEAD $(BASEDIR)
	$(Q)GZIP=-9 tar -rf $(APPNAME)-$(MODE)-$(TARGET_OS)-$(TARGET_ARCH).tar.gz $(APPNAME)$(EXE_EXT) $(BASEDIR)/textures/*.lua $(BASEDIR)/pics/*.*
	@echo "Create zip archive of $(APPNAME)"
	$(Q)git archive -9 --format zip -o $(APPNAME)-$(MODE)-$(TARGET_OS)-$(TARGET_ARCH).zip HEAD $(BASEDIR)
	$(Q)zip -qu9 $(APPNAME)-$(MODE)-$(TARGET_OS)-$(TARGET_ARCH).zip $(APPNAME)$(EXE_EXT) $(BASEDIR)/textures/*.lua $(BASEDIR)/pics/*.*

sourcearchive:
	$(Q)git archive --format=tar --prefix=caveexpress-$(VERSION)-source/ HEAD | bzip2 -9 > caveexpress-$(VERSION)-source.tar.bz2
