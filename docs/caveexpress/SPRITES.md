# Sprites, atlases, and drawing

This is the data path from a PNG to something the game draws and collides with. Use it together with [EDITOR.md](EDITOR.md) (shape editor) and [MAPS.md](MAPS.md) (scripted props).

## Files

| File | Role |
| --- | --- |
| `contrib/assets/png/caveexpress*.tps` | TexturePacker 3.9.4 sources |
| `contrib/assets/png/caveexpress/` | Source PNGs (entities, dust, waste, …) |
| `base/caveexpress/textures/caveexpress-*-small.lua` / `-big.lua` | Atlas UV frames (`trimmedwidth` is the drawn pixel size) |
| `base/caveexpress/sprites.lua` | Animation, layers, tile size, collision shapes |
| `base/caveexpress/entities.lua` | Physics width/height for named entity types (`player`, `npcman`, …) |

Regenerate entity atlases after changing PNGs:

```sh
./textureatlas contrib/assets/png/caveexpress-entity.tps
```

See `src/tools/textureatlas/README.md` and the [README tools section](../../README.md#textureatlas).

## `sprites.lua`

Each key is a sprite id used by `map:addTile(...)`, `addTileRuntime(...)`, or an animation name (`player-empty`, `npc-man-idle`).

```lua
["dust"] = {
    fps = 8,
    width = 2.0,
    height = 1.4,
    frames = {
        {}, -- back
        {}, -- middle
        { "dust-01", "dust-02", --[[ ... ]] "dust-08", }, -- front
    },
},
```

### `width` / `height`

These are **tile units for placement and physics**, not the drawn image size.

- A tile cell is 1×1. On the small atlas, 1 tile is 64 reference pixels.
- The image comes from the atlas (`trimmedwidth` × `trimmedheight`).
- A 64×57 `player-empty` image is about 1.0 × 0.89 tiles on screen even if `entities.lua` says `player` is `0.94 × 0.87`.
- Changing `width`/`height` does **not** scale the bitmap. It only moves the box the sprite is aligned into.

### Layers (`frames`)

The `frames` table is always `{ back, middle, front }`. The client draws every entity once per layer, in this order:

1. `LAYER_BACK`
2. `LAYER_MIDDLE` (default for the player and walking NPCs)
3. `LAYER_FRONT`
4. `LAYER_FRONT_1`, `LAYER_FRONT_2`

A sprite that should cover characters (dust, waste piles, idea bubbles) belongs on **front**. The machine (`player-empty`) and `npc-man` sit on **middle**, so a front-layer dust cloud draws over them. That is how a cutscene can place the machine under a puff and then remove the dust.

Empty layer tables (`{}`) draw nothing on that pass.

### Animation

- `fps` and optional `delays` drive frame timing.
- Map tiles animate continuously. The player only advances frames while moving unless the script changes the animation.
- Animations loop unless the `Animation` object is created with `loop = false`. Most gameplay ids loop.

## Alignment and coordinates

CaveExpress Y **increases downward** (gravity). “Above” is a smaller Y.

Most world objects use `ENTITY_ALIGN_LOWER_LEFT`:

- Physics / `getPos()` is the **body center**.
- The bitmap is centered horizontally on that X.
- The bitmap sits on the bottom of the physics box (`centerY + halfHeight`).

For a ground line at `y = 5`:

| Entity | Typical height | Center Y to stand on the ground |
| --- | --- | --- |
| Player / parked machine | 0.87 | `5.0 - 0.435` |
| `npc-man` | 0.548 | `5.0 - 0.274` |

A 1×1 map tile at grid `(gx, gy)` has its **visual baseline** at `gy + height`. `addTile("waste", 3.0, 3.5)` with `height = 1.5` sits on `y = 5`.

`ENTITY_ALIGN_MIDDLE_CENTER` is used for a few flying/swimming things (fruit, fish, pterodactyls): the sprite is centered on `getPos()`.

## Collision polygons

`polygons` in `sprites.lua` are Box2D fixtures. The in-game **Shapes** tool (see [EDITOR.md](EDITOR.md#sprite-shape-editor)) edits them.

- Stored vertices are **tile units**.
- Lua writes them as tile × 100 (`100` = 1 tile).
- Y+ in the editor is up (converted when applying Lua).
- At most 8 vertices per convex polygon.
- The first string in each polygon table is user data (`""`, `"solid"`, `"lava"`, …).
- Circles are defined separately and are read-only in the editor.

`entities.lua` `width`/`height` still define the default body size when no sprite shape is used.

## Practical cutscene notes

- To hide something, park it off the left edge (negative X). `getPos()` then stays there until you `setPos` again — pin scripted objects every frame if physics can move them.
- `sprites.lua` `width`/`height` will not enlarge a small PNG. Scale the source image and regenerate the atlas.
- If a prop must appear “from” a cloud, spawn the prop **while the front-layer animation is still playing**, then remove the prop’s cover sprite. Do not remove the cover first.
