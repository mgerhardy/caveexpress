#!/bin/bash
set -euo pipefail

BUILD_TYPE=Release
if [ "${ANDROID_RELEASE:-ON}" = "OFF" ]; then
	BUILD_TYPE=Debug
fi

ANDROID_ABI="${ANDROID_ABI:-arm64-v8a}"
ANDROID_PLATFORM="${ANDROID_PLATFORM:-android-21}"
GAMES="${GAMES:-caveexpress cavepacker}"

DIR=$(dirname "$(readlink -f "$0")")
ROOT=$(readlink -f "$DIR/../..")
BUILD_DIR="${ANDROID_BUILD_DIR:-$ROOT/cp-build-android}"
SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$ROOT/build/android_sdk}}"

ndk_usable() {
	local ndk="${1:-}"
	[ -n "$ndk" ] && [ -f "$ndk/build/cmake/android.toolchain.cmake" ]
}

if ! ndk_usable "${ANDROID_NDK_HOME:-}" && ! ndk_usable "${ANDROID_NDK:-}"; then
	if ndk_usable "$SDK_ROOT/ndk/28.2.13676358"; then
		export ANDROID_NDK_HOME="$SDK_ROOT/ndk/28.2.13676358"
		export ANDROID_SDK_ROOT="$SDK_ROOT"
	fi
fi
if ! ndk_usable "${ANDROID_NDK_HOME:-}" && ! ndk_usable "${ANDROID_NDK:-}"; then
	echo "Android NDK/SDK not found — downloading via setup-android.sh"
	ANDROID_SDK_ROOT="$SDK_ROOT" "$DIR/setup-android.sh"
	# setup-android.sh prints export lines; apply the last known layout
	if [ -f "$SDK_ROOT/android-env.sh" ]; then
		# shellcheck disable=SC1091
		source "$SDK_ROOT/android-env.sh"
	else
		export ANDROID_SDK_ROOT="$SDK_ROOT"
		export ANDROID_HOME="$SDK_ROOT"
		export ANDROID_NDK_HOME="$SDK_ROOT/ndk/28.2.13676358"
	fi
fi
export ANDROID_NDK_HOME="${ANDROID_NDK_HOME:-${ANDROID_NDK:-}}"
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$SDK_ROOT}}"
export ANDROID_HOME="${ANDROID_HOME:-$ANDROID_SDK_ROOT}"
export PATH="${ANDROID_SDK_ROOT}/platform-tools:${ANDROID_NDK_HOME}:${PATH}"

if ! ndk_usable "$ANDROID_NDK_HOME"; then
	echo "ANDROID_NDK_HOME does not look like an NDK: $ANDROID_NDK_HOME" >&2
	exit 1
fi
echo "Using ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
echo "Using ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"

mkdir -p "$BUILD_DIR"
cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
	-DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
	-DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/android-toolchain.cmake" \
	-DANDROID_ABI="${ANDROID_ABI}" \
	-DANDROID_PLATFORM="${ANDROID_PLATFORM}" \
	-DANDROID_STL=c++_static \
	-DTOOLS=OFF \
	-DUNITTESTS=OFF

TARGETS=""
for game in $GAMES; do
	TARGETS="$TARGETS $game"
done
# shellcheck disable=SC2086
cmake --build "$BUILD_DIR" --target $TARGETS

if [ "${ANDROID_SKIP_APK:-}" = "1" ]; then
	exit 0
fi

if [ -z "${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}" ]; then
	echo "ANDROID_SDK_ROOT not set — native libraries are in $BUILD_DIR/android/<game>/jniLibs" >&2
	exit 0
fi
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"
printf 'sdk.dir=%s\n' "$ANDROID_SDK_ROOT" > "$ROOT/android-project/local.properties"

for game in $GAMES; do
	cmake --build "$BUILD_DIR" --target "android-${game}-apk"
done
