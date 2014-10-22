ANDROID_PROJECT=android-project
ifneq ($(findstring $(TARGET_OS), android ouya),)
ifeq ($(DEBUG),)
ANT_TARGET          := release
ANT_INSTALL_TARGET  := installr
MANIFEST_DEBUGGABLE := true
APPLICATION_MK      := APP_OPTIM := debug
else
ANT_TARGET          := debug
ANT_INSTALL_TARGET  := installd
MANIFEST_DEBUGGABLE := false
APPLICATION_MK      :=
endif
ifeq ($(Q),)
NDK_VERBOSE:="V=1"
else
NDK_VERBOSE:=
endif

ifeq ($(TARGET_OS),android)
PERMISSIONS := <uses-permission android:name="com.android.vending.BILLING" />
ifeq ($(HD_VERSION),1)
META_DATA =
else
META_DATA = <meta-data android:value="true" android:name="ADMOB_ALLOW_LOCATION_FOR_ADS" />
endif
#META_DATA += <meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/app_id" />
#META_DATA += <meta-data android:name="com.google.android.gms.appstate.APP_ID" android:value="@string/app_id" />
META_DATA += <meta-data android:name="com.google.android.gms.version" android:value="@integer/google_play_services_version" />
ANDROID_REFERENCED_LIBS := android.library.reference.1=google-play-services_lib
else
PERMISSIONS :=
META_DATA :=
ANDROID_REFERENCED_LIBS :=
endif

ifeq ($(HD_VERSION),1)
ICON := $(APPNAME)hd-icon.png
else
ICON := $(APPNAME)-icon.png
endif

.PHONY: clean-android-libs
clean-android-libs:
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/.project
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/assets/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/bin/
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/build.xml
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/gen/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/libs/
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/local.properties
	$(Q)rm -f $(ANDROID_PROJECT)/google-play-services_lib/proguard-project.txt
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/res/drawable-ldpi/
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/res/layout/

.PHONY: clean-android
clean-android: clean-android-libs
	@echo "===> ANDROID [clean project]"
	$(Q)rm -rf $(ANDROID_PROJECT)/assets
	$(Q)rm -rf $(ANDROID_PROJECT)/bin
	$(Q)rm -rf $(ANDROID_PROJECT)/obj
	$(Q)rm -rf $(ANDROID_PROJECT)/gen
	$(Q)rm -f $(ANDROID_PROJECT)/local.properties
	$(Q)rm -f $(ANDROID_PROJECT)/default.properties
	$(Q)rm -f $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)rm -f $(ANDROID_PROJECT)/build.xml
	$(Q)rm -f $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)rm -f $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)rm -f $(SRCDIR)/Android.mk

android-update-project: $(ANDROID_PROJECT)/local.properties $(ANDROID_PROJECT)/build.xml $(ANDROID_PROJECT)/AndroidManifest.xml

$(ANDROID_PROJECT)/AndroidManifest.xml: $(SRCDIR)/Android.mk.in $(ANDROID_PROJECT)/AndroidManifest.xml.in $(ANDROID_PROJECT)/jni/Application.mk.in $(ANDROID_PROJECT)/strings.xml.in $(CONFIG_H)-config.h
	$(Q)cp $(ANDROID_PROJECT)/AndroidManifest.xml.in $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@DEBUGGABLE@/$(MANIFEST_DEBUGGABLE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's,@PERMISSIONS@,$(PERMISSIONS),g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's,@META_DATA@,$(META_DATA),g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@VERSION@/$(VERSION)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@VERSION_CODE@/$(VERSION_CODE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@JAVA_PACKAGE@/$(JAVA_PACKAGE)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@MIN_ANDROID_SDK@/$(MIN_ANDROID_SDK)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/AndroidManifest.xml
	$(Q)cp $(ANDROID_PROJECT)/strings.xml.in $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/res/values/strings.xml
	$(Q)cp $(ANDROID_PROJECT)/jni/Application.mk.in $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)sed -i 's/@APPLICATION_MK@/$(APPLICATION_MK)/g' $(ANDROID_PROJECT)/jni/Application.mk
	$(Q)cp $(SRCDIR)/Android.mk.in $(SRCDIR)/Android.mk
	$(Q)sed -i 's/@APPNAME@/$(APPNAME)/g' $(SRCDIR)/Android.mk
	$(Q)sed -i 's/@NETWORKING@/$(NETWORKING)/g' $(SRCDIR)/Android.mk
	$(Q)sed -i 's#@OWN_CFLAGS@#$(SDL_NET_CFLAGS)#g' $(SRCDIR)/Android.mk
	$(Q)sed -i 's/import org.caveexpress.*R;/import $(JAVA_PACKAGE).R;/g' $(ANDROID_PROJECT)/src/org/base/game/GameHelper.java
	$(Q)sed -i 's/import org.caveexpress.*R;/import $(JAVA_PACKAGE).R;/g' $(ANDROID_PROJECT)/src/org/base/game/GameHelperUtils.java

android-update-sdk-version:
	$(Q)cp $(ANDROID_PROJECT)/default.properties.in $(ANDROID_PROJECT)/default.properties
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/default.properties
	$(Q)cp $(ANDROID_PROJECT)/project.properties.in $(ANDROID_PROJECT)/project.properties
	$(Q)sed -i 's/@TARGET_ANDROID_SDK@/$(TARGET_ANDROID_SDK)/g' $(ANDROID_PROJECT)/project.properties
	$(Q)sed -i 's/@ANDROID_REFERENCED_LIBS@/$(ANDROID_REFERENCED_LIBS)/g' $(ANDROID_PROJECT)/project.properties

$(ANDROID_PROJECT)/build.xml: $(ANDROID_PROJECT)/build.xml.in $(CONFIG_H)-config.h
	$(Q)cp $(ANDROID_PROJECT)/build.xml.in $(ANDROID_PROJECT)/build.xml
	$(Q)sed -i 's/@APPNAME_FULL@/$(APPNAME_FULL)/g' $(ANDROID_PROJECT)/build.xml
	$(Q)sed -i 's:@EXCLUDE_PACKAGE_PATHS@:$(EXCLUDE_JAVA_PACKAGE_PATHS):g' $(ANDROID_PROJECT)/build.xml

define ANDROID_PACKAGE
$(Q)cd $(ANDROID_PROJECT); SDK=`android list sdk | grep $(1) | awk -F'-' ' { print $$1 }'`; [ -n "$$SDK" ] && (SDK2=`android list sdk --all | grep $(1) | awk -F'-' ' { print $$1 }'`; android update sdk -a -u -s -t $$SDK2) || echo
endef

android-install-dependencies:
	$(call ANDROID_PACKAGE,"SDK Platform Android 3.2")
	$(call ANDROID_PACKAGE,"SDK Platform Android 4.1.2")
	#$(call ANDROID_PACKAGE,"Google Play Billing Library")
	$(call ANDROID_PACKAGE,"Google Play services")

$(ANDROID_PROJECT)/local.properties: android-install-dependencies $(ANDROID_PROJECT)/AndroidManifest.xml $(ANDROID_PROJECT)/build.xml $(ANDROID_PROJECT)/google-play-services_lib/build.xml
	@echo "===> ANDROID [update project]"
	$(Q)cd $(ANDROID_PROJECT) && android update project -p . -t android-13

$(ANDROID_PROJECT)/google-play-services_lib/build.xml:
	@echo "===> ANDROID [update lib project]"
	$(Q)cd $(ANDROID_PROJECT) && android update lib-project -t android-13 --path google-play-services_lib

.PHONY: android-copy-assets
android-copy-assets: data
	@echo "===> CP [copy assets]"
	$(Q)rm -rf $(ANDROID_PROJECT)/assets
	$(Q)rm -rf $(ANDROID_PROJECT)/libs
	$(Q)rm -rf $(ANDROID_PROJECT)/google-play-services_lib/bin/res/crunch
	$(Q)mkdir -p $(ANDROID_PROJECT)/libs
	$(Q)mkdir -p $(ANDROID_PROJECT)/assets/$(BASEROOT)
	$(Q)cp -rf $(BASEDIR) $(ANDROID_PROJECT)/assets/$(BASEROOT)
	$(Q)rm -f $(ANDROID_PROJECT)/assets/$(BASEDIR)/maps/test*
	$(Q)rm -f $(ANDROID_PROJECT)/assets/$(BASEDIR)/maps/empty*
	$(Q)for i in hdpi ldpi mdpi xhdpi; do mkdir -p $(ANDROID_PROJECT)/res/drawable-$${i}; cp -f contrib/$(ICON) $(ANDROID_PROJECT)/res/drawable-$${i}/icon.png; done
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then cp contrib/installer/ouya/ouya_icon.png $(ANDROID_PROJECT)/res/drawable-xhdpi; fi
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then mkdir -p $(ANDROID_PROJECT)/res/raw && cp ~/ouyakey.der $(ANDROID_PROJECT)/res/raw/key.der; fi
	$(Q)if [ "$(TARGET_OS)" = "ouya" ]; then cp contrib/installer/ouya/*.jar $(ANDROID_PROJECT)/libs; fi
	$(Q)if [ "$(TARGET_OS)" = "android" ]; then mkdir -p $(ANDROID_PROJECT)/libs && cp contrib/installer/android/*.jar $(ANDROID_PROJECT)/libs; fi

.PHONY: android-build
android-build: android-update-sdk-version
	@echo "===> NDK [native build]"
	$(Q)cd $(ANDROID_PROJECT) && NDK_CCACHE=ccache ndk-build -j 4 $(NDK_VERBOSE)
	@echo "===> ANT [java build]"
	$(Q)cd $(ANDROID_PROJECT) && ant $(ANT_TARGET)

.PHONY: android-install
android-install:
	$(Q)cd $(ANDROID_PROJECT) && ant $(ANT_INSTALL_TARGET)

android-emulator-create:
	android create avd -n CaveExpress -t android-13

android-emulator-start:
	emulator -avd CaveExpress
# -scale 96dpi -dpi-device 160

android-emulator-run: android-emulator-start android-install android-run

.PHONY: android-all
.NOTPARALLEL:
android-all: clean-android-libs android-update-project android-copy-assets android-build android-uninstall android-install

.PHONY: android
.NOTPARALLEL:
android: clean-android-libs android-update-project android-copy-assets android-build

android-backtrace:
	adb logcat | ndk-stack -sym $(ANDROID_PROJECT)/obj/local/armeabi

android-run:
	adb shell am start -n $(JAVA_PACKAGE)/$(JAVA_PACKAGE).$(APPNAME_FULL)

android-uninstall:
	$(Q)cd $(ANDROID_PROJECT) && ant uninstall
endif

release-android:
	./configure --enable-ccache --target-os=android --enable-release && $(MAKE) clean-android clean && $(MAKE)
	cp $(ANDROID_PROJECT)/bin/*-release.apk $(APPNAME_FULL)-release.apk
	./configure --enable-ccache --target-os=android --enable-release --enable-hd && $(MAKE) clean-android clean && $(MAKE)
	cp $(ANDROID_PROJECT)/bin/*-release.apk $(APPNAME_FULL)-hd-release.apk
	./configure --enable-ccache --target-os=ouya --enable-release && $(MAKE) clean-android clean && $(MAKE)
	cp $(ANDROID_PROJECT)/bin/*-release.apk $(APPNAME_FULL)-ouya-release.apk

android-setup:
	$(Q)ARCH=x86_64; \
	NDK_VERSION=r10b; \
	SDK_VERSION=20140702; \
	[ $(TARGET_ARCH) = "i386" ] && ARCH=x86; \
	echo "Downloading the ndk..."; \
	wget --quiet --continue http://dl.google.com/android/ndk/android-ndk32-$$NDK_VERSION-linux-$$ARCH.tar.bz2; \
	echo "Extracting the ndk..."; \
	tar -xjf android-ndk32-$$NDK_VERSION-linux-$$ARCH.tar.bz2 -C ~/; \
	echo "Downloading the sdk..."; \
	wget --quiet --continue http://dl.google.com/android/adt/adt-bundle-linux-$$ARCH-$$SDK_VERSION.zip; \
	echo "Extracting the sdk..."; \
	ARCHIVE=`readlink -f adt-bundle-linux-$$ARCH-$$SDK_VERSION.zip`; \
	cd ~; \
	unzip -o -qq $$ARCHIVE; \
	echo "Configure paths..."; \
	echo "export ANDROID_SDK=~/adt-bundle-linux-$$ARCH-$$SDK_VERSION/sdk" >> ~/.bashrc; \
	echo "export ANDROID_NDK=~/android-ndk-$$NDK_VERSION" >> ~/.bashrc; \
	echo "export NDK_ROOT=\$$ANDROID_NDK" >> ~/.bashrc; \
	echo "export PATH=\$$PATH:\$$ANDROID_NDK:\$$ANDROID_SDK/tools:\$$ANDROID_SDK/platform-tools" >> ~/.bashrc;
