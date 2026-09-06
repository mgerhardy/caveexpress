#!/bin/bash

# Package a macOS .app into a compressed DMG.
# Finder AppleScript layout is skipped: it fails on headless CI (no TCC / Finder).

set -euo pipefail

if [ $# -lt 2 ]; then
	echo "usage: $0 <app-name> <version> [src-dir] [build-dir]" >&2
	exit 1
fi

APP_NAME="$1"
VERSION="$2"
SRC_DIR="${3:-.}"
BUILD_DIR="${4:-.}"

SRC_DIR="$(cd "$SRC_DIR" && pwd)"
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"

find_app() {
	local name="$1.app"
	local candidate
	for candidate in \
		"$BUILD_DIR/$name" \
		"$SRC_DIR/$name" \
		"$BUILD_DIR/Release/$name" \
		"$BUILD_DIR/src/caveexpress/main/Release/$name" \
		"$BUILD_DIR/src/caveexpress/main/$name"
	do
		if [ -d "$candidate" ]; then
			printf '%s\n' "$candidate"
			return 0
		fi
	done
	# CMAKE_RUNTIME_OUTPUT_DIRECTORY may place the bundle at the repo root.
	# macOS /usr/bin/find has no -quit; take the first match.
	find "$SRC_DIR" "$BUILD_DIR" -name "$name" -type d | head -n 1
}

APP_BUNDLE="$(find_app "$APP_NAME")"
if [ -z "${APP_BUNDLE:-}" ] || [ ! -d "$APP_BUNDLE" ]; then
	echo "Could not find ${APP_NAME}.app under ${SRC_DIR} or ${BUILD_DIR}" >&2
	find "$SRC_DIR" "$BUILD_DIR" -name '*.app' -type d || true
	exit 1
fi

echo "Using app bundle: $APP_BUNDLE"

# Ad-hoc signature only (no Apple Developer ID). Needed on Apple Silicon after
# fixup_bundle rewrites the Mach-O; CMake/Ninja does not sign anything itself.
if command -v codesign >/dev/null 2>&1; then
	codesign --force --deep --sign - "$APP_BUNDLE"
fi

VOL_NAME="${APP_NAME} ${VERSION}"
DMG_FINAL="${BUILD_DIR}/${VOL_NAME}.dmg"
STAGING_DIR="${BUILD_DIR}/Install"

rm -rf "$STAGING_DIR" "$DMG_FINAL"
mkdir -p "$STAGING_DIR"
cp -R "$APP_BUNDLE" "$STAGING_DIR/"
ln -s /Applications "$STAGING_DIR/Applications"

BACKGROUND_SRC="${SRC_DIR}/contrib/installer/osx/background.png"
if [ -f "$BACKGROUND_SRC" ]; then
	mkdir -p "$STAGING_DIR/.background"
	cp "$BACKGROUND_SRC" "$STAGING_DIR/.background/Background.png"
fi

hdiutil create \
	-volname "$VOL_NAME" \
	-srcfolder "$STAGING_DIR" \
	-ov \
	-fs HFS+ \
	-format UDZO \
	-imagekey zlib-level=9 \
	"$DMG_FINAL"

rm -rf "$STAGING_DIR"
echo "Created ${DMG_FINAL}"
