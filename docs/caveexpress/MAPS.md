# Map Lua scripts

Maps are Lua files in `base/caveexpress/maps/` (player copies go under the SDL user data dir). The in-game editor writes `getName` and `initMap` on every save. Hand-written logic belongs in `onMapLoaded`, `onUpdate`, and helpers — see [EDITOR.md](EDITOR.md#map-scripts-and-lua).

This page covers the **runtime** script API used by cutscenes and scripted maps. The intro movie `intro-movie-package` is the worked example.

## Map file shape

```lua
function getName()
    return "Intro Movie"
end

function initMap()
    local map = Map.get()
    map:addTile("tile-background-01", 0, 0)
    map:addTile("tile-ground-04", 0, 5)
    map:addCave("tile-cave-01", 1, 4, "npc-man", 60000)
    map:addStartPosition("2", "3")
    map:setSetting("width", "14")
    map:setSetting("height", "8")
    map:setSetting("packagetransfercount", "3")
    -- ...
end

function onMapLoaded()
    local map = Map.get()
    -- runtime only: spawn, hide the player, lock input
end

function onUpdate(dt)
    -- dt is milliseconds since the last tick
end
```

`initMap` / `getName` run when the map is parsed. `onMapLoaded` / `onUpdate` run only after the server has a live `Map` (they error if you call spawn APIs during `initMap`).

## Coordinates

- Grid and physics use **tile units**. The origin is the top-left of the map.
- **Y increases downward.** Gravity pulls toward larger Y. “Above” is a smaller Y.
- `addTile(sprite, x, y)` places a tile with **lower-left** alignment. A tile of height `h` at grid Y `y` visually sits on `y + h`.
- `entity:getPos()` / `setPos(x, y)` use the **body center**.
- Ground at row 5 has its top surface at `y = 5`. A player of height `0.87` stands at `y = 5.0 - 0.435`.

See [SPRITES.md](SPRITES.md) for how sprite `width`/`height` relates to the drawn PNG.

## Settings

`map:setSetting(name, value)` in `initMap`. Values are strings.

| Setting | Meaning |
| --- | --- |
| `width` / `height` | Grid size |
| `theme` | `rock`, `ice`, `jungle`, `desert` |
| `points` | Score for a normal finish |
| `referencetime` | Par time in seconds (stars on playable maps) |
| `packagetransfercount` | Packages that must reach a shredder for `map:isDone()` |
| `npctransfercount` | Friendly NPCs to deliver |
| `npcs` | Friendly spawn cap |
| `flyingnpc` / `fishnpc` | Random enemies |
| `gravity` / `wind` | Physics |
| `waterheight`, `waterchangespeed`, `waterrisingdelay`, `waterfallingdelay` | Water |
| `sideborderfail` | Fail when touching the side border |
| `tutorial` | Do not increment the global “maps finished” counter |
| `cutscene` | Hide the HUD. The script owns the ending (see below). |
| `introwindow` | Optional help window id; empty string skips it |

### Cutscenes (`cutscene=true`)

- Input is usually locked with `map:setInputEnabled(false)` in `onMapLoaded`.
- `map:isDone()` still becomes true when the package (or NPC) quota is met. The game **does not** fade out or show the star screen until the script calls `map:finish()`.
- `map:finish()` marks the campaign map as completed (so Continue starts the **next** map), skips the “how many stars” window, and loads the next campaign map after a short fade.
- There is no human score for a fully scripted intro. Stars are stored as 0.
- Skip (action / mapped skip key) should call `map:consumeSkip()` then `map:finish()` so the same path runs.

Playable tutorial maps keep `tutorial=true` but leave `cutscene` off so they still show stars.

## `Map` methods (runtime)

Available as `Map.get()` inside `onMapLoaded` / `onUpdate`.

| Method | Notes |
| --- | --- |
| `finish()` | Force the win (`forceComplete`). Required to end a cutscene. |
| `isDone()` | Quota met, or `finish()` was called. |
| `getTime()` | Milliseconds since the map started. |
| `getSize()` | Width, height. |
| `setInputEnabled(bool)` / `isInputEnabled()` | Lock flying / drop. Enabling input clears latched skip. |
| `isKeyPressed(name)` | `"left"`, `"right"`, `"up"`, `"down"`, `"skip"`. Skip is also latched from the action button while input is locked. |
| `consumeSkip()` | Clear the skip latch after handling it. |
| `message(text, ms)` | HUD banner. |
| `getGravity()` / `calculateVelocity(...)` | Physics helpers. |
| `getPlayer([index])` | 1-based. |
| `getEntity(id)` | By entity id. |
| `getCaveCount()` / `getCave(n)` | 1-based cave list. |
| `getWaterHeight()` / `setWaterHeight(y)` | |
| `spawnPackage(x, y)` | Loose package. |
| `getPackages()` | Array of package entities. |
| `getPackageCount()` / `getPackage(n)` | |
| `getPackageTarget()` | Shredder. |
| `getDeliveredPackageCount()` / `getCollectedPackageCount()` / `getPackageDeliveryGoal()` | |
| `spawnPackageNPC(caveIndex, type)` | Script-driven dumper (`npc-woman`, …). Does **not** auto-dump on idle. |
| `spawnFriendlyNPC(caveIndex, type, returnToCaveOnIdle)` | Taxi / story villager. |
| `spawnNPC(type, x, y)` | World NPCs (flying, fish, …) — not cave villagers. |
| `addTileRuntime(sprite, x, y [, angle])` | Decal / prop after load (`dust`, `waste`, `idea`). |
| `removeTileAt(x, y)` / `replaceTile(sprite, x, y)` / `rebuildPlatforms()` | Terrain edits. |
| `removeEntity(ent or id)` | Not the player. |

`isKeyPressed` is also a global function.

## Entity methods

Handles are userdata. After `remove()` they fail `isValid()`.

### All entities

| Method | Notes |
| --- | --- |
| `isValid()` | False after remove or despawn. |
| `getId()` / `getType()` | |
| `getPos()` | Returns `x, y` (center). |
| `setPos(x, y)` | On cave NPCs this also updates the “home” position. |
| `getVelocity()` / `setVelocity(vx, vy)` | |
| `applyImpulse` / `applyForce` | |
| `getGravityScale()` / `setGravityScale(s)` | `0` pins flying cutscene motion. |
| `getGravity()` | |
| `getState()` / `setState(n)` | Raw state int. Prefer `setIdle` / `setMoving` for NPCs. |
| `remove()` | Illegal on the player. |
| `isPlayer()` / `isNpc()` / `isCave()` | |
| `setAnimation(name)` | `"idle"`, `"empty"`, `"flying"`, `"crashed"`, … |

### Player

| Method | Notes |
| --- | --- |
| `accelerate(dir)` / `resetAcceleration([dir])` | `resetAcceleration` forces the idle anim; use `setAnimation` after it if you still want flying. |
| `setInvulnerable(ms)` | Ignores hitpoint loss. **Landing rumble still fires** if the impact is over `damagethreshold` (default 3). Soft-land or keep `setGravityScale(0)` in intros. |
| `drop()` | Drop the carried package. |
| `getCollectedPackageCount()` / `getCollectedPackages()` | |

### NPCs

| Method | Notes |
| --- | --- |
| `setMoving(x)` | Walk to that X. Directional types play a turn, then a **delayed** walk. |
| `setIdle()` | Cancels the pending walk timer, zeros velocity, plays the idle animation. Call this (and keep calling it) while an idea bubble is up. |
| `isIdle()` | |
| `setDone()` | Friendly NPC finished (removed). |
| `returnToCave()` | Walk back to the last `setPos` / spawn home. |
| `dropPackage()` / `leavePackage()` | Package NPCs only. `leavePackage` also starts the walk home. |

`NPCPackage` spawned from the script does not auto-leave a package when idle (that aborted multi-drop scenes). You must `dropPackage` / `returnToCave` yourself. When a scripted dumper is idle **at its home X**, it despawns.

Friendly NPCs with `returnToCaveOnIdle=true` walk home whenever they go idle. For a cutscene, pass `false` and drive `setMoving` yourself.

`setIdle()` must cancel the walk timer. If you only zero velocity, a scheduled `move()` still fires and the walk cycle keeps playing.

### Caves

| Method | Notes |
| --- | --- |
| `getLightState()` / `setLightState(bool)` | |
| `setNextSpawn(ms)` / `setRespawnPossible(bool [, type])` | Cutscenes usually disable respawn. |
| `spawnCaveNPC()` / `getCaveNumber()` | |

### Packages / shredder

| Method | Notes |
| --- | --- |
| `isPackage()` / `isCollected()` / `isArrived()` / `isDelivered()` / `isDestroyed()` | |
| `isPulling()` / `getPullingPackage()` | On the package target. |

Delivered packages drop out of `countPackages()`. A script that waits for “three on the floor” should also look at `getDeliveredPackageCount()`.

## Scripted flight

To move the machine without player input:

1. `setInputEnabled(false)`
2. `setGravityScale(0)` so it does not fall
3. `setVelocity` toward the next waypoint; after `resetAcceleration()` set `setAnimation("flying")` again
4. Keep `setInvulnerable` refreshed
5. Do **not** restore gravity while still in the air. A hover-to-ground drop hits hard, rumble-shakes the screen, and looks like a crash.

Landing: last waypoint on the ground line, speed well below `damagethreshold`, then `setPos` + `setVelocity(0,0)` + `setGravityScale(0)`.

## Runtime tiles (dust, waste, idea)

`addTileRuntime("dust", x, y)` creates a map tile after load. Put cover FX on the **front** layer in `sprites.lua` so they draw over the player and NPCs ([SPRITES.md](SPRITES.md#layers-frames)).

Typical reveal:

1. Play dust.
2. Hide or place the machine/NPC **under** that tile.
3. Remove the dust tile when the puff has covered them.

Removing the dust first makes the machine appear from empty air.

## Campaign

Add the map id (filename without `.lua`) to `base/caveexpress/campaigns/*.lua`:

```lua
c:addMaps("intro-movie-package")
c:addMaps("introducing-01-package")
```

After a cutscene `finish()`, Continue / the automatic hand-off starts the next unlocked map. Replaying the campaign from a reset plays the intro again.
