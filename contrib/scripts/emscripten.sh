#!/bin/bash
# Configure and build Emscripten targets. Output (*.html, *.js, *.wasm, *.data)
# is written to the repository root. See docs/EMSCRIPTEN.md.
#
# Atlas PNGs under base/<game>/pics/ are gitignored. CI generates them with a
# host textureatlas build first; locally we do the same when ./textureatlas exists.

set -euo pipefail
DIR=$(dirname "$(readlink -f "$0")")
ROOT=$(readlink -f "$DIR/../..")

generate_atlases() {
	local atlas="$ROOT/textureatlas"
	if [ ! -x "$atlas" ]; then
		echo "No $atlas — skip atlas generation (pics must already exist under base/*/pics/)"
		return 0
	fi
	echo "Generating texture atlases into base/*/pics/"
	(cd "$ROOT" && "$atlas" contrib/assets/png/caveexpress-*.tps contrib/assets/png/cavepacker-*.tps)
}

mkdir -p "$ROOT/cp-build-emscripten"
cd "$ROOT/cp-build-emscripten"
pwd
generate_atlases
emcmake cmake "$ROOT" -DCMAKE_BUILD_TYPE=Release -DUNITTESTS=OFF -DTOOLS=OFF
if [ $# -eq 0 ]; then
	cmake --build . --parallel
else
	cmake --build . --parallel --target "$@"
fi
