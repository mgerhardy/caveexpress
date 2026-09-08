#!/bin/bash
# Android SDK/NDK/emulator setup, APK build, and local emulator run.
# Versions match .github/workflows/main.yml (NDK r28c, platform 34, build-tools 34.0.0).
#
#   ./contrib/scripts/android.sh          # build APKs (CI default)
#   ./contrib/scripts/android.sh setup    # SDK, NDK, emulator, AVD — skips existing files
#   ./contrib/scripts/android.sh run      # setup + build (host ABI) + emulator + install + launch
#
# Prefer the Makefile: `make android-setup`, `make android`, `make android-run`.
set -euo pipefail

DIR=$(dirname "$(readlink -f "$0")")
ROOT=$(readlink -f "$DIR/../..")

# NDK r28c — same as nttld/setup-ndk in CI
NDK_VERSION="${ANDROID_NDK_VERSION:-28.2.13676358}"
CMDLINE_VERSION="${ANDROID_CMDLINE_TOOLS:-11076708}"
COMPILE_SDK="${ANDROID_COMPILE_SDK:-34}"
BUILD_TOOLS_VER="${ANDROID_BUILD_TOOLS:-34.0.0}"
NATIVE_PLATFORM="${ANDROID_PLATFORM:-android-21}"
SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$ROOT/build/android_sdk}}"
BUILD_DIR="${ANDROID_BUILD_DIR:-$ROOT/cp-build-android}"
ANDROID_GAME="${ANDROID_GAME:-caveexpress}"

usage() {
	cat <<EOF
Usage: $(basename "$0") [command] [cmake-target]

Commands:
  setup          Install SDK, NDK, emulator, system image, AVD (skips existing)
  build          Configure + build native libs and debug APKs (default; CI)
  emulator       Start the AVD if no Android device is connected
  wait           Wait until the emulator/device has finished booting
  install        adb install APK(s); starts the emulator if needed
  start          Launch ANDROID_GAME (default: caveexpress)
  logs           Follow logcat for the game (crashes, SDL, ActivityManager)
  run            setup + build (host ABI) + emulator + install + start
  stop           Shut down a running emulator
  cmake-target   cmake --build <android-dir> --target <name>
  help           Show this help

Environment:
  ANDROID_SDK_ROOT / ANDROID_HOME   SDK path (default: $ROOT/build/android_sdk)
  ANDROID_NDK_HOME / ANDROID_NDK    NDK path (default: SDK ndk/$NDK_VERSION)
  ANDROID_ABI                       build default: arm64-v8a; run default: host ABI
  ANDROID_PLATFORM                  native API (default: android-21)
  BUILDTYPE / ANDROID_RELEASE       Debug or Release native libs
  GAMES                             games to package (default: caveexpress cavepacker)
  ANDROID_GAME                      game to launch (default: caveexpress)
  ANDROID_AVD                       AVD name (default: caveexpress-<abi>)
  ANDROID_SKIP_APK=1                native .so only
  ANDROID_EMULATOR_HEADLESS=1       no emulator window
  ANDROID_LOGS_DUMP=1               logs: print the buffer and exit
EOF
}

host_abi() {
	case "$(uname -m)" in
		x86_64) echo x86_64 ;;
		aarch64|arm64) echo arm64-v8a ;;
		i386|i686) echo x86 ;;
		*) echo x86_64 ;;
	esac
}

ndk_usable() {
	local ndk="${1:-}"
	[ -n "$ndk" ] && [ -f "$ndk/build/cmake/android.toolchain.cmake" ]
}

resolve_ndk() {
	if ndk_usable "${ANDROID_NDK_HOME:-}"; then
		return 0
	fi
	if ndk_usable "${ANDROID_NDK:-}"; then
		export ANDROID_NDK_HOME="$ANDROID_NDK"
		return 0
	fi
	if ndk_usable "$SDK_ROOT/ndk/$NDK_VERSION"; then
		export ANDROID_NDK_HOME="$SDK_ROOT/ndk/$NDK_VERSION"
		return 0
	fi
	local d
	for d in "$SDK_ROOT"/ndk/*; do
		if ndk_usable "$d"; then
			export ANDROID_NDK_HOME="$d"
			return 0
		fi
	done
	return 1
}

export_android_env() {
	export ANDROID_SDK_ROOT="$SDK_ROOT"
	export ANDROID_HOME="$SDK_ROOT"
	resolve_ndk || true
	export PATH="$SDK_ROOT/cmdline-tools/latest/bin:$SDK_ROOT/platform-tools:$SDK_ROOT/emulator:${ANDROID_NDK_HOME:-}:$PATH"
}

write_env_file() {
	local env_file="$SDK_ROOT/android-env.sh"
	{
		echo "export ANDROID_SDK_ROOT=$SDK_ROOT"
		echo "export ANDROID_HOME=$SDK_ROOT"
		echo "export ANDROID_NDK_HOME=${ANDROID_NDK_HOME:-$SDK_ROOT/ndk/$NDK_VERSION}"
		echo "export PATH=\"\$ANDROID_SDK_ROOT/cmdline-tools/latest/bin:\$ANDROID_SDK_ROOT/platform-tools:\$ANDROID_SDK_ROOT/emulator:\$ANDROID_NDK_HOME:\$PATH\""
	} > "$env_file"
}

sdk_pkg_path() {
	local pkg="$1"
	case "$pkg" in
		platform-tools) printf '%s\n' "$SDK_ROOT/platform-tools/adb" ;;
		emulator) printf '%s\n' "$SDK_ROOT/emulator/emulator" ;;
		ndk\;*) printf '%s\n' "$SDK_ROOT/ndk/${pkg#ndk;}" ;;
		platforms\;*) printf '%s\n' "$SDK_ROOT/platforms/${pkg#platforms;}" ;;
		build-tools\;*) printf '%s\n' "$SDK_ROOT/build-tools/${pkg#build-tools;}" ;;
		system-images\;*)
			local rest="${pkg#system-images;}"
			printf '%s\n' "$SDK_ROOT/system-images/${rest//;//}"
			;;
		*) printf '%s\n' "$SDK_ROOT/${pkg//;//}" ;;
	esac
}

sdk_pkg_present() {
	local path
	path="$(sdk_pkg_path "$1")"
	[ -e "$path" ]
}

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		echo "Missing command: $1" >&2
		exit 1
	fi
}

check_java() {
	if ! command -v java >/dev/null 2>&1; then
		echo "JDK 17+ is required (CI uses Temurin 17). Install e.g. openjdk-17-jdk." >&2
		exit 1
	fi
}

ensure_cmdline_tools() {
	local sdkmanager="$SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
	if [ -x "$sdkmanager" ]; then
		return 0
	fi
	need_cmd unzip
	local fetch sdk_file tmp
	sdk_file="commandlinetools-linux-${CMDLINE_VERSION}_latest.zip"
	tmp=$(mktemp -d)
	echo "Downloading Android cmdline-tools ${CMDLINE_VERSION}"
	if command -v wget >/dev/null 2>&1; then
		wget --no-verbose -O "$tmp/$sdk_file" "https://dl.google.com/android/repository/${sdk_file}"
	else
		need_cmd curl
		curl -fsSL -o "$tmp/$sdk_file" "https://dl.google.com/android/repository/${sdk_file}"
	fi
	mkdir -p "$SDK_ROOT/cmdline-tools"
	rm -rf "$SDK_ROOT/cmdline-tools/latest"
	unzip -q "$tmp/$sdk_file" -d "$tmp"
	mv "$tmp/cmdline-tools" "$SDK_ROOT/cmdline-tools/latest"
	rm -rf "$tmp"
}

accept_licenses() {
	local sdkmanager="$SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
	set +e
	yes | "$sdkmanager" --sdk_root="$SDK_ROOT" --licenses >/dev/null 2>&1
	set -e
}

install_sdk_packages() {
	local sdkmanager="$SDK_ROOT/cmdline-tools/latest/bin/sdkmanager"
	local missing=()
	local pkg
	for pkg in "$@"; do
		if sdk_pkg_present "$pkg"; then
			echo "SDK package already present: $pkg"
		else
			missing+=("$pkg")
		fi
	done
	if [ ${#missing[@]} -eq 0 ]; then
		return 0
	fi
	echo "Installing SDK packages: ${missing[*]}"
	accept_licenses
	"$sdkmanager" --sdk_root="$SDK_ROOT" "${missing[@]}"
}

setup_sdk_ndk() {
	mkdir -p "$SDK_ROOT"
	ensure_cmdline_tools
	export_android_env
	local ndk_pkg="ndk;$NDK_VERSION"
	if resolve_ndk; then
		echo "Using existing NDK: $ANDROID_NDK_HOME"
		install_sdk_packages \
			"platforms;android-${COMPILE_SDK}" \
			"build-tools;${BUILD_TOOLS_VER}" \
			platform-tools
	else
		install_sdk_packages \
			"platforms;android-${COMPILE_SDK}" \
			"build-tools;${BUILD_TOOLS_VER}" \
			platform-tools \
			"$ndk_pkg"
		resolve_ndk
	fi
	if ! resolve_ndk; then
		echo "ANDROID_NDK_HOME does not look like an NDK: ${ANDROID_NDK_HOME:-unset}" >&2
		exit 1
	fi
	write_env_file
	export_android_env
}

sysimage_pkg() {
	printf 'system-images;android-%s;google_apis;%s\n' "$COMPILE_SDK" "$(host_abi)"
}

avd_name() {
	printf '%s\n' "${ANDROID_AVD:-caveexpress-$(host_abi)}"
}

setup_emulator() {
	local image avd avdmanager
	image="$(sysimage_pkg)"
	avd="$(avd_name)"
	install_sdk_packages emulator "$image"
	export_android_env
	avdmanager="$SDK_ROOT/cmdline-tools/latest/bin/avdmanager"
	if [ -f "$HOME/.android/avd/${avd}.ini" ]; then
		echo "AVD already exists: $avd"
		return 0
	fi
	echo "Creating AVD $avd ($image)"
	# "no" = do not create a custom hardware profile
	printf 'no\n' | "$avdmanager" create avd \
		--name "$avd" \
		--package "$image" \
		--device pixel_6
}

device_connected() {
	local adb="$SDK_ROOT/platform-tools/adb"
	[ -x "$adb" ] || return 1
	"$adb" start-server >/dev/null 2>&1 || true
	"$adb" devices 2>/dev/null | awk 'NR>1 && $2=="device" {found=1} END {exit found?0:1}'
}

cmd_wait() {
	local adb="$SDK_ROOT/platform-tools/adb"
	local timeout="${ANDROID_BOOT_TIMEOUT:-300}"
	local boot
	echo "Waiting for Android device/emulator..."
	"$adb" wait-for-device
	while [ "$timeout" -gt 0 ]; do
		boot="$("$adb" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')"
		if [ "$boot" = "1" ]; then
			echo "Device booted"
			return 0
		fi
		sleep 2
		timeout=$((timeout - 2))
	done
	echo "Timed out waiting for the emulator to boot. See emulator log if started by this script." >&2
	exit 1
}

cmd_emulator() {
	setup_sdk_ndk
	setup_emulator
	if device_connected; then
		echo "Android device already connected"
		return 0
	fi
	local emu="$SDK_ROOT/emulator/emulator"
	local avd args=()
	avd="$(avd_name)"
	if [ ! -x "$emu" ]; then
		echo "Emulator binary not found: $emu" >&2
		exit 1
	fi
	args=(-avd "$avd" -netdelay none -netspeed full)
	if [ -z "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ] || [ "${ANDROID_EMULATOR_HEADLESS:-}" = "1" ]; then
		args+=(-no-window -no-audio -gpu swiftshader_indirect)
	fi
	if [ ! -e /dev/kvm ]; then
		echo "Warning: /dev/kvm is missing — the emulator will be slow" >&2
		args+=(-accel off -gpu swiftshader_indirect)
	fi
	mkdir -p "$BUILD_DIR"
	echo "Starting emulator AVD $avd"
	"$emu" "${args[@]}" >"$BUILD_DIR/emulator.log" 2>&1 &
	echo $! >"$BUILD_DIR/emulator.pid"
	cmd_wait
}

cmd_stop() {
	local adb="$SDK_ROOT/platform-tools/adb"
	if [ -x "$adb" ]; then
		"$adb" start-server >/dev/null 2>&1 || true
		"$adb" emu kill >/dev/null 2>&1 || true
	fi
	if [ -f "$BUILD_DIR/emulator.pid" ]; then
		kill "$(cat "$BUILD_DIR/emulator.pid")" >/dev/null 2>&1 || true
		rm -f "$BUILD_DIR/emulator.pid"
	fi
	pkill -f "qemu-system.*$(avd_name)" >/dev/null 2>&1 || true
	# Wait until adb no longer lists an emulator
	local n=0
	while [ "$n" -lt 20 ]; do
		if ! device_connected; then
			echo "Emulator stopped"
			return 0
		fi
		sleep 1
		n=$((n + 1))
	done
	echo "Emulator may still be shutting down" >&2
}

games_to_build() {
	printf '%s\n' "${GAMES:-caveexpress cavepacker}"
}

native_build_type() {
	if [ -n "${BUILDTYPE:-}" ]; then
		printf '%s\n' "$BUILDTYPE"
	elif [ "${ANDROID_RELEASE:-ON}" = "OFF" ]; then
		printf 'Debug\n'
	else
		printf 'Release\n'
	fi
}

cache_abi() {
	local cache="$BUILD_DIR/CMakeCache.txt"
	if [ ! -f "$cache" ]; then
		return 0
	fi
	sed -n 's/^CMAKE_ANDROID_ARCH_ABI:[^=]*=//p' "$cache" | head -1
}

configure_native() {
	local abi="${ANDROID_ABI:-arm64-v8a}"
	local existing
	existing="$(cache_abi || true)"
	if [ -n "$existing" ] && [ "$existing" != "$abi" ]; then
		echo "ABI changed ($existing -> $abi), reconfiguring $BUILD_DIR"
		rm -rf "$BUILD_DIR"
	fi
	need_cmd cmake
	need_cmd ninja
	mkdir -p "$BUILD_DIR"
	echo "Using ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
	echo "Using ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
	echo "Using ANDROID_ABI=$abi"
	cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
		-DCMAKE_BUILD_TYPE="$(native_build_type)" \
		-DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/android-toolchain.cmake" \
		-DANDROID_ABI="$abi" \
		-DANDROID_PLATFORM="$NATIVE_PLATFORM" \
		-DANDROID_STL=c++_static \
		-DTOOLS=OFF \
		-DUNITTESTS=OFF
}

write_local_properties() {
	printf 'sdk.dir=%s\n' "$ANDROID_SDK_ROOT" > "$ROOT/android-project/local.properties"
}

apk_path() {
	printf '%s\n' "$ROOT/android-project/app/build/outputs/apk/$1/debug/app-$1-debug.apk"
}

activity_for_game() {
	case "$1" in
		caveexpress) printf 'org.caveexpress/org.caveexpress.CaveExpress\n' ;;
		cavepacker) printf 'org.cavepacker/org.cavepacker.CavePacker\n' ;;
		*)
			echo "Unknown game: $1 (expected caveexpress or cavepacker)" >&2
			exit 1
			;;
	esac
}

cmd_build() {
	check_java
	setup_sdk_ndk
	configure_native
	local targets="" game
	for game in $(games_to_build); do
		targets="$targets $game"
	done
	# shellcheck disable=SC2086
	cmake --build "$BUILD_DIR" --target $targets
	if [ "${ANDROID_SKIP_APK:-}" = "1" ]; then
		return 0
	fi
	write_local_properties
	for game in $(games_to_build); do
		cmake --build "$BUILD_DIR" --target "android-${game}-apk"
	done
}

cmd_install() {
	local adb game apk
	setup_sdk_ndk
	if ! device_connected; then
		cmd_emulator
	else
		cmd_wait
	fi
	adb="$SDK_ROOT/platform-tools/adb"
	for game in $(games_to_build); do
		apk="$(apk_path "$game")"
		if [ ! -f "$apk" ]; then
			echo "APK not found: $apk (build it with: $0 build)" >&2
			exit 1
		fi
		echo "Installing $apk"
		"$adb" install -r "$apk"
	done
}

cmd_start() {
	local adb="$SDK_ROOT/platform-tools/adb"
	setup_sdk_ndk
	if ! device_connected; then
		echo "No Android device connected" >&2
		exit 1
	fi
	"$adb" shell am start -n "$(activity_for_game "$ANDROID_GAME")"
}

cmd_logs() {
	setup_sdk_ndk
	if ! device_connected; then
		echo "No Android device connected. Start one with: make android-emulator" >&2
		exit 1
	fi
	local adb="$SDK_ROOT/platform-tools/adb"
	local pkg="org.${ANDROID_GAME}"
	# SDL native logs, Java activity, ART/JNI aborts, linker errors
	local filters=(
		SDL:V
		DEBUG:I
		AndroidRuntime:V
		libc:V
		"${ANDROID_GAME}:V"
		"${pkg}:V"
		ActivityManager:I
		ActivityTaskManager:I
	)
	if [ "${ANDROID_LOGS_DUMP:-}" = "1" ]; then
		"$adb" logcat -d -v threadtime -t "${ANDROID_LOGS_LINES:-500}" "${filters[@]}" '*:S'
		return 0
	fi
	echo "Following logcat for ${pkg} (Ctrl-C to stop)"
	echo "Recent crash/fatal lines:"
	"$adb" logcat -d -v threadtime -t 400 | grep -E 'F/DEBUG|E/AndroidRuntime|FATAL EXCEPTION|UnsatisfiedLinkError|F/libc' || true
	echo "---- live ----"
	"$adb" logcat -v threadtime "${filters[@]}" '*:S'
}

cmd_run() {
	ANDROID_ABI="${ANDROID_ABI:-$(host_abi)}"
	GAMES="${GAMES:-$ANDROID_GAME}"
	export GAMES
	setup_sdk_ndk
	setup_emulator
	cmd_build
	cmd_emulator
	cmd_install
	cmd_start
}

cmd_setup() {
	check_java
	setup_sdk_ndk
	setup_emulator
	write_env_file
	echo "Android setup complete."
	echo "  ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
	echo "  ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
	echo "  AVD=$(avd_name)"
	echo "Source $SDK_ROOT/android-env.sh or use: make android / make android-run"
}

cmd_cmake_target() {
	local target="${1:-}"
	if [ -z "$target" ]; then
		echo "cmake-target requires a target name" >&2
		exit 1
	fi
	check_java
	setup_sdk_ndk
	configure_native
	write_local_properties
	cmake --build "$BUILD_DIR" --target "$target"
}

cmd="${1:-build}"
if [ $# -gt 0 ]; then
	shift
fi

case "$cmd" in
	setup) cmd_setup ;;
	build) cmd_build ;;
	emulator) cmd_emulator ;;
	wait) export_android_env; cmd_wait ;;
	install) cmd_install ;;
	start) cmd_start ;;
	logs) cmd_logs ;;
	run) cmd_run ;;
	stop) cmd_stop ;;
	cmake-target) cmd_cmake_target "${1:-}" ;;
	help|-h|--help) usage ;;
	*)
		echo "Unknown command: $cmd" >&2
		usage >&2
		exit 1
		;;
esac
