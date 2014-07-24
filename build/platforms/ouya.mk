MIN_ANDROID_SDK=16
TARGET_ANDROID_SDK=16
CONFIG_H:=android
ifeq ($(APPNAME),cavepacker)
    EXCLUDE_JAVA_PACKAGE_PATHS:=**/org/caveexpress*/** **/org/base/game/**
else
    EXCLUDE_JAVA_PACKAGE_PATHS:=**/org/cavepacker*/** **/org/base/game/**
endif
