# CaveExpress Map Editor

This guide explains how to create and edit CaveExpress maps with the built-in editor.

## Opening the editor

- From the main menu, click **Editor**.
- From the command line: `caveexpress -ui_push editor`
- From the in-game console (`Shift+Tab`): `ui_push editor`

To open a named map directly: `loadmap introducing-01-package`

Unsaved changes are confirmed before leaving the editor, starting a new map, or loading another map.

## Window layout

| Area | Contents |
| --- | --- |
| Top toolbar | New, Save, Save to game data, Save & Go, Play from here, Undo, Redo, Fit, Script, Shapes, Help, and the Place / Remove / Select / Fill tools |
| **Palette** (left) | **Tiles**, **Entities**, **Maps** |
| **Map** (center) | Map canvas |
| **Properties** / **Layers** (right) | Map settings, the selected item, and layer visibility |

These are Dear ImGui dock panels. Drag a title bar to undock or restack them. Drag the split between panels to resize. The first-run layout is Palette left, map in the center, Properties and Layers stacked on the right.

The filename in the toolbar shows a `*` when there are unsaved changes.

**Tiles** and **Entities** are separate edit modes. Placement, picking, highlighting, and erase only affect the kind of item that belongs to the active tab. Caves, gates, pressure plates, and terrain are tiles. Packages, stones, dinosaurs, trees, and the player start are entities.

## Creating a playable map

1. Click **New** (or start from an existing map on the **Maps** tab).
2. Set **File** (on-disk name, no `.lua`) and **Title** (shown in-game).
3. Set **Width** and **Height**. Minimum size is 6x4.
4. Choose a **Theme** (rock, ice, jungle, desert). The tile palette is filtered to that theme.
5. On the **Tiles** tab, paint background, then ground/rock, caves, and a package target (shredder).
6. On the **Entities** tab, place the **player** start and at least one package (or rely on caves to spawn them).
7. Set **The amount of packages to deliver** to the number of packages the player must drop at the shredder.
8. Place a start position that has flying room. **Save & Go** to test.

A typical first map needs: open background cells, a solid landing surface, a cave or package emitter, a shredder, a player start, and `packagetransfercount` greater than zero.

## Tools

The toolbar radio buttons choose the left-mouse action:

| Tool | Left mouse button |
| --- | --- |
| **Place tile** | Paint the active brush (tile or entity) and select that cell |
| **Remove tile** | Erase items that belong to the active tab |
| **Select tile** | Pick the item under the cursor into the brush (active tab only) |
| **Fill** | Flood-fill empty cells or matching tiles of the active layer with the brush |

Right mouse button always erases, regardless of the selected tool.

## Mouse

| Input | Action |
| --- | --- |
| Left click / drag | Paint or place. Also selects the cell so Properties show that item. |
| Shift + left click / drag | Rectangle select. Ctrl+C / Ctrl+V copies and pastes the active tab. Arrow keys nudge. |
| Alt + left or middle click | Pick whatever is on top (any tab). |
| Right click / drag | Erase. On the **Tiles** tab this removes tiles only (background, rock, caves, gates, ...). On the **Entities** tab this removes entities only (packages, stones, NPCs, player start, ...). Holding the button does not punch through to the other kind. |
| Middle click | Pick the item under the cursor (active tab). |
| Middle drag | Pan the view. |
| Space + left drag | Pan the view. |
| Mouse wheel | Zoom toward the cursor. |

A green ghost preview follows the cursor when a brush is selected. The yellow outline is the selected item used by Properties, not merely the hovered cell.

## Keyboard

These shortcuts are ignored while you are typing in a text field, except **Ctrl+S**.

| Key | Action |
| --- | --- |
| **Ctrl+S** | Save |
| **Ctrl+Z** | Undo |
| **Ctrl+Y** | Redo |
| **F** | Fit the map in the canvas |
| **G** | Toggle the grid |
| **F1** | Toggle the in-editor help panel |
| **Space** (cursor on canvas) | Rotate the **selected** tile if it is rotatable, or flip a selected directional NPC. Otherwise rotates the brush. Angle / facing is shown on the ghost and in the toolbar. |
| **Ctrl+C** / **Ctrl+V** | Copy / paste the rectangle selection |
| **Arrow keys** | Nudge the selection (or the highlighted tile) |
| **Delete** / **Backspace** | Remove the selected item or the rectangle of the active tab |
| **Esc** | Close Script, Shapes, Help, or the unsaved-changes dialog; otherwise leave the editor |

## Tiles tab

Click a tile in the palette to make it the brush, then paint on the canvas. Use the filter box to search by sprite id.

### Common tile kinds

| Kind | Role |
| --- | --- |
| Background | Open air the player flies through. Paint these first. |
| Ground / rock | Solid collision. Ground is the landing surface; rock fills volume. |
| Cave | Clients live here. Caves spawn packages (or a named NPC) after **Npc delay**. |
| Package target (shredder) | Deliver packages here to score. |
| Geyser | Updraft that can lift the plane. |
| Bridge / liane | Overlay decoration on open background. |
| Window / cave art | Visual detail on background. |
| Gate / pressure plate | Linked trigger pair (see below). |

### Placement rules

The editor rejects some illegal placements instead of painting:

- Bridges and lianes need a background tile covering their cells.
- Bridges also need a ground or bridge neighbor on the left or right.
- Thin hanging ground (`ground-05`, `ground-06`) and ground ledge ends need empty space beneath them.

### Layers

The **Layers** tab shows or hides:

- background
- solid (rock, ground, gates, plates)
- foreground (bridges)
- decoration (lianes)
- emitter (entities)

Hiding a layer also prevents selecting and erasing items on that layer. **Show Grid** toggles the cell grid (also **G**).

## Entities tab

Click a sprite in the grid (tooltip shows the type name), then left-click the map to place it. Right-click removes only entities and the player start. Background and other tiles stay in place. Use the filter box to search by entity type name.

| Entity | Notes |
| --- | --- |
| `player` | Start position. You can place more than one. |
| `item-package` / `item-package-ice` | Packages to deliver. Ice packages belong on ice-themed maps. |
| `item-stone` | Drop on dinosaurs or trees. |
| `item-apple` / `item-banana` / `item-egg` | Pickup items (health, strength, invulnerability). |
| `item-bomb` | Explosive pickup. |
| `npc-walking` / `npc-blowing` / `npc-mammut` | Ground dinosaurs. Space flips facing. |
| `tree` | Drop a stone on it for fruit. |

**Emitter amount** and **Emitter delay** (Properties) apply to the next placed emitter, or to the selected emitter if one is highlighted. Directional NPCs have **Faces right**. Blowing dinosaurs also have **Blow strength** and **Wind size** (`strength=` / `size=` in Lua). Selected emitters can be nudged with sub-tile **X** / **Y**.

Cave signs, `dust`, `waste`, and `idea` are on the Tiles tab (decoration / cutscene props).

Fish and pterodactyls are not placed as entities. Enable them with the **Activate the fish spawn** and **Activate the pterodactyls spawn** checkboxes so they appear according to map water and flying space.

Friendly cave NPCs (`npc-man`, `npc-woman`, `npc-grandpa`) are assigned on a selected cave tile (Properties: **Spawn NPC** and **Spawn delay ms**). The brush default is **Cave NPC (brush)** / **Npc delay**. Taxi/rescue maps also need **Friendly NPCs to deliver** (`npctransfercount`).

## Properties

Always bound to the last clicked cell, not the hover ghost.

### Map

| Field | Meaning |
| --- | --- |
| File | Saved as `<file>.lua` (see [Saving](#saving-and-testing)). |
| Title | In-game map name (`getName()`). |
| Width / Height | Grid size. Values are clamped to 6-160 by 4-120. |
| +W / -W / +H / -H | Grow or shrink the map and shift existing tiles. |
| Points | Score awarded for finishing. |
| Reference time in seconds | Par time used for stars. |
| Gravity | Default `9.81`. |
| Wind | Horizontal force. Use with care; some wind maps also set `sideborderfail`. |
| Waterheight | Water surface from the bottom of the map, in tiles. Drawn as a blue overlay. `0` means no water. |
| The amount of packages to deliver | Packages that must reach a shredder to win. |
| Friendly NPCs to deliver | `npctransfercount` for taxi/rescue maps (needs two caves). |
| Friendly NPC spawn cap | `npcs` — how many villagers caves may keep alive. |
| First flying/fish spawn delay | `initialspawntime` in ms (`0` = engine random). |
| Geyser initial delay | `geyserinitialdelay` in ms (all geysers). |
| Activate the pterodactyls spawn | Random flying enemy. |
| Activate the fish spawn | Random fish in water. |
| Fail on side border | `sideborderfail`. |
| Tutorial / Cutscene | `tutorial` and `cutscene` (cutscenes skip the star screen). |
| Intro window | Built-in help window (`intropackage`, `introflying`, …). Empty skips it. |
| Water change / rising / falling delay | Rising or falling water. |
| Npc delay / Cave NPC (brush) | Defaults for newly placed caves. |
| Theme | Switches the palette. A confirm dialog appears if the remap would leave themed tiles unchanged. |
| Seed + Auto + Make playable | Generate a random map. **Make playable** adds a cave, shredder, package, and start if missing. |
| Add to campaign | Append `c:addMaps("file")` to a campaign Lua file. Lists campaigns that already contain the map; **Remove** drops the line. **Create campaign** writes a new `campaigns/*.lua`. |
| Check layout | Runs the same reachability checks as `MapValidator` (covered caves, unreachable caves/targets). |
| Extra settings | Raw key/value pairs that have no dedicated control (still written on save). |
| Keep handwritten initMap | On save, copy `initMap` from the existing file instead of regenerating tiles. |

### Gates and pressure plates

Select a plate or gate (Tiles tab). Properties then show **Trigger link**:

- **Link id** - both sides of a pair must share the same id (for example `door1`).
- **Pick gate target** - click a plate, press the button, then click the gate.
- Plate **Required weight** (default `700`) and **Hold ms**.
- Gate **Open amount** (`0`-`1`).

A yellow line is drawn between linked partners.

Start positions are listed under Properties (edit X/Y, **Play** from that pad, or delete). Place extra starts with the **player** entity.

Alt+drag the blue water line on the canvas to change water height. Wind draws arrows along the top of the map. **Show all trigger links** draws every plate–gate pair; unpaired items have **Go**.

## Maps tab

Lists installed and previously saved maps. Click a name to load it (you are asked to confirm if the current map is dirty). The filter box searches by filename.

## Saving and testing

| Action | Result |
| --- | --- |
| **Save** / **Ctrl+S** | Writes the Lua map file to the user data directory and reloads the map list. Validation warnings can be ignored with Save anyway. |
| **Save to game data** | Writes `base/caveexpress/maps/<file>.lua` in the project / install data dir. |
| **Save & Go** | Saves to user data and starts the map immediately. |
| **Play from here** | Saves and starts with the machine at the view center (god mode for 10 minutes). |

Maps are written to the user data directory:

`~/.local/share/caveexpress/base/caveexpress/maps/<file>.lua`

(on other platforms, the equivalent SDL preference path). Built-in maps live in `base/caveexpress/maps/` next to the game data. Use **Save to game data** to write there.

After saving, the map appears on the **Maps** tab and can be started with `map <file>` from the console.

**Add to campaign** on Properties appends `c:addMaps("mymap")` to the chosen `base/caveexpress/campaigns/*.lua` file. **Create campaign** writes a new file with `icon-campaign-rock` and the current map already listed. Campaign icon, achievement, and map order stay in that Lua file.

## Sprite shape editor

**Shapes** opens a tool for editing collision polygons and circles defined in `sprites.lua`. **Write sprites.lua** patches that sprite's `polygons` / `circles` tables in the game-data file. Copy Lua remains available. How those shapes, layers, and atlas sizes relate to drawing is in [SPRITES.md](SPRITES.md).

1. Click **Shapes**, or select a tile/entity first so that sprite is preselected.
2. Pick any sprite from the filterable list.
3. The sprite image is shown centered on the origin. The blue rectangle is the sprite's tile size (`width` x `height`).
4. Left-click empty canvas to append a vertex (line strip). Left-drag a red dot to move it. Right-click a red dot to delete it.
5. **New polygon** / **Delete polygon** switch among multiple fixtures (see `item-banana-idle`). **User data** is the first string in each polygon table (`""`, `"solid"`, `"lava"`, ...).
6. Copy the generated `polygons = { ... },` block and paste it into the sprite entry in `sprites.lua`. **Paste** + **Apply Lua** loads a definition from the clipboard or the text box.

Hold **Ctrl** while placing or dragging to snap to 1 Lua unit (0.01 tiles). Box2D allows at most 8 vertices per convex polygon; the editor warns if a shape is concave or too large. **New circle** adds a circle; drag the center to move it, drag the rim to resize.

Edits update the in-memory sprite definition for the current session. **Write sprites.lua** (or a restart after a manual paste) keeps them.

## Map scripts and Lua

**Script** opens a Lua editor for logic that is kept across saves: `onMapLoaded`, `onUpdate`, and helper functions. Line numbers, find/replace-all, a short API list, and **Insert cutscene lock** / **Insert skip-to-finish** snippets are in that window. `getName` and `initMap` are regenerated from the editor data on every save unless **Keep handwritten initMap** is checked.

The full runtime API, coordinates, and cutscene rules are in [MAPS.md](MAPS.md). Sprite layers, atlas sizes, and alignment are in [SPRITES.md](SPRITES.md).

Useful settings (most are also on Properties):

| Setting | Meaning |
| --- | --- |
| `packagetransfercount` | Packages required (also in Properties). |
| `npctransfercount` | Friendly NPCs to deliver to a target cave. |
| `waterchangespeed` | Rising/falling water. |
| `waterrisingdelay` / `waterfallingdelay` | Delay before water starts moving. |
| `sideborderfail` | Fail if you touch the side border (used on some wind maps). |
| `flyingnpc` / `fishnpc` | Also in Properties. |
| `tutorial` | Do not increment the global maps-finished counter. |
| `cutscene` | Hide the HUD. `map:finish()` skips the star screen and starts the next campaign map. |
| `introwindow` | Help window id; use `""` to skip. |

Example from a built-in map:

```lua
map:addStartPosition("6", "3")
map:addCave("tile-cave-01", 6, 3, "npc-man", 5000)
map:addEmitter("item-package", 2.2, 0, 1, 200, "")
map:setSetting("packagetransfercount", "2")
```

## Suggested workflow

1. New map -> set file name, title, size, theme.
2. Fill background, then carve flying space and landings with ground/rock.
3. Place at least one cave or package, and one shredder.
4. Switch to **Entities**, place the player, packages, and any dinosaurs or trees.
5. Tune points, reference time, water, and enemy spawn flags.
6. **Save & Go**, fly the route, come back and adjust.
7. Add the map to a campaign when it is ready.

Use **Tiles** when editing terrain so a right-click cannot delete a package by mistake. Use **Entities** when dressing the map so a right-click cannot delete the background under an item.
