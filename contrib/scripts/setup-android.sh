#!/bin/bash

TARGET_ARCH=`uname -p | sed -e s/i.86/i386/`
ARCH=x86_64
#NDK_VERSION=r10d
NDK_VERSION=r14
SDK_VERSION=r24.4.1
[ $TARGET_ARCH = "i386" ] && ARCH=x86
echo "Downloading the ndk..."
echo "==> http://dl.google.com/android/ndk/android-ndk-$NDK_VERSION-linux-$ARCH.bin"
wget --quiet --continue https://dl.google.com/android/repository/android-ndk-$NDK_VERSION-linux-$ARCH.zip
#wget --quiet --continue http://dl.google.com/android/ndk/android-ndk-$NDK_VERSION-linux-$ARCH.bin
echo "Extracting the ndk..."
#chmod +x android-ndk-$NDK_VERSION-linux-$ARCH.bin
#./android-ndk-$NDK_VERSION-linux-$ARCH.bin -o/home/$USER/
echo "==> target: /home/$USER/"
unzip -qq -n -d ~/ android-ndk-$NDK_VERSION-linux-$ARCH.zip
echo "Downloading the sdk..."
echo "==> http://dl.google.com/android/android-sdk_$SDK_VERSION-linux.tgz"
wget --quiet --continue http://dl.google.com/android/android-sdk_$SDK_VERSION-linux.tgz
echo "Extracting the sdk..."
echo " ==> target: /home/$USER/"
tar -xzf android-sdk_$SDK_VERSION-linux.tgz -C ~/
echo "Configure paths..."
echo "export ANDROID_SDK=~/android-sdk-linux" >> ~/.bashrc
echo "export ANDROID_NDK=~/android-ndk-$NDK_VERSION" >> ~/.bashrc
echo "export NDK_ROOT=\$ANDROID_NDK" >> ~/.bashrc
echo "export PATH=\$PATH:\$ANDROID_NDK:\$ANDROID_SDK/tools:\$ANDROID_SDK/platform-tools" >> ~/.bashrc
