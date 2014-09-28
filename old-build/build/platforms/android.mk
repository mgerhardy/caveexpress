MIN_ANDROID_SDK=10
TARGET_ANDROID_SDK=13
ifeq ($(APPNAME),cavepacker)
    EXCLUDE_JAVA_PACKAGE_PATHS:=**/org/caveexpress*/** **/org/*ouya/**
else
    EXCLUDE_JAVA_PACKAGE_PATHS:=**/org/cavepacker*/** **/org/*ouya/**
endif
