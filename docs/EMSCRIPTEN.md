# Emscripten (web) builds

CaveExpress and CavePacker share the same HTML5 shell, preload layout, and local test server. Output files land in the **repository root**: `caveexpress.html` / `cavepacker.html` plus matching `.js`, `.wasm`, and `.data`.

## Build

Needs [emsdk](https://emscripten.org/docs/getting_started/downloads.html). CI uses emsdk **4.0.5**.

```sh
source ~/emsdk/emsdk_env.sh   # or wherever emsdk lives
./contrib/scripts/emscripten.sh caveexpress
./contrib/scripts/emscripten.sh cavepacker
```

`emscripten.sh` generates atlas PNGs with `./textureatlas` (same as CI’s host `build-data` step), configures `cp-build-emscripten/` with `emcmake`, and runs `cmake --build` for the targets you pass. The linker preloads `base/<game>/` into MEMFS at `/base/<game>/`, including `pics/`. Those atlas PNGs are gitignored; without them the web build has no textures.

The GL3 frontend is the HTML5 default (`config.lua` `isHTML5()`). Desktop CavePacker also defaults to `opengl3` except Android, which stays on the SDL renderer.

## Why a local HTTP server

Browsers will not fetch `.wasm` or the packed `.data` file from a `file://` URL. Serve the repo root (or the directory that contains the four files) over HTTP.

`contrib/scripts/emscripten-serve.js` is a small Node static server. It sets `Cache-Control: no-cache` and COOP/COEP headers (harmless here, required if threads are ever enabled):

```sh
# from the repository root, after the build
node contrib/scripts/emscripten-serve.js
# optional: --port 8080 --dir /path/to/output
```

Open:

* http://127.0.0.1:8000/caveexpress.html
* http://127.0.0.1:8000/cavepacker.html

Pass in-game commands after `?` (spaces as `+` or `%20`), for example:

* http://127.0.0.1:8000/cavepacker.html?-map+tutorial0002

`npx --yes serve -p 8000` is the same idea if you already use that package; it does not send COOP/COEP.

## Image loading

SDL_image loads atlas PNGs from MEMFS with `IMG_Load_RW`. The browser’s async image decoder cannot do that, so the shell sets `Module.noImageDecoding = true` and decoding goes through libpng (`sdl2_image:formats=png` in `cmake/emscripten.cmake`).

HTML5 uses the dummy sound engine, so missing music files in the console are expected.

## Config in the browser

Settings persist in IndexedDB (`IDBFS` mounted at `/user_data/`). An old stored `frontend=sdl` overrides `config.lua`. Clear site data for localhost, or run `set frontend opengl3` in the in-game console (`SHIFT+TAB`).
