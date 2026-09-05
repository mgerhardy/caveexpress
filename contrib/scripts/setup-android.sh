#!/bin/bash
# Install a local Android SDK + NDK for native / Gradle builds.
# CI uses nttld/setup-ndk and android-actions/setup-android instead.
set -euo pipefail

SDK_ROOT="${ANDROID_SDK_ROOT:-$PWD/build/android_sdk}"
CMDLINE_VERSION="${ANDROID_CMDLINE_TOOLS:-11076708}"
PLATFORM="${ANDROID_PLATFORM_PKG:-platforms;android-34}"
BUILD_TOOLS="${ANDROID_BUILD_TOOLS_PKG:-build-tools;34.0.0}"
NDK_PKG="${ANDROID_NDK_PKG:-ndk;28.2.13676358}"

mkdir -p "$SDK_ROOT"
cd "$SDK_ROOT"

if [ ! -x cmdline-tools/latest/bin/sdkmanager ]; then
	sdk_file="commandlinetools-linux-${CMDLINE_VERSION}_latest.zip"
	wget --no-verbose "https://dl.google.com/android/repository/${sdk_file}"
	rm -rf cmdline-tools/latest
	mkdir -p cmdline-tools
	unzip -q "$sdk_file" -d /tmp/android-cmdline-$$
	mv /tmp/android-cmdline-$$/cmdline-tools cmdline-tools/latest
	rm -rf /tmp/android-cmdline-$$ "$sdk_file"
fi

SDKMANAGER="$PWD/cmdline-tools/latest/bin/sdkmanager"
set +e
yes | "$SDKMANAGER" --sdk_root="$SDK_ROOT" --licenses >/dev/null
set -e
"$SDKMANAGER" --sdk_root="$SDK_ROOT" \
	"$PLATFORM" \
	"$BUILD_TOOLS" \
	platform-tools \
	"$NDK_PKG"

ENV_FILE="$SDK_ROOT/android-env.sh"
{
	echo "export ANDROID_SDK_ROOT=$SDK_ROOT"
	echo "export ANDROID_HOME=$SDK_ROOT"
	echo "export ANDROID_NDK_HOME=$SDK_ROOT/ndk/28.2.13676358"
	echo "export PATH=\"\$ANDROID_SDK_ROOT/platform-tools:\$ANDROID_NDK_HOME:\$PATH\""
} > "$ENV_FILE"
cat "$ENV_FILE"
