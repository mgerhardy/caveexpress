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
| Top toolbar | New, Save, Save & Go, Undo, Redo, Fit, Script, Help, and the Place / Remove / Select tools |
| Left tabs | **Tiles**, **Entities**, **Maps** |
| Center | Map canvas |
| Right tabs | **Properties** (map settings and the selected item) and **Layers** |

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

Right mouse button always erases, regardless of the selected tool.

## Mouse

| Input | Action |
| --- | --- |
| Left click / drag | Paint or place. Also selects the cell so Properties show that item. |
| Shift + left click | Pick the item under the cursor into the brush (same as Select tool). |
| Right click / drag | Erase. On the **Tiles** tab this removes tiles only (background, rock, caves, gates, ...). On the **Entities** tab this removes entities only (packages, stones, NPCs, player start, ...). Holding the button does not punch through to the other kind. |
| Middle click | Pick the item under the cursor. |
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
| **Space** (cursor on canvas) | Rotate the brush. Directional entities flip left/right. Rotatable tiles step by their sprite rotation increment (often 90 degrees). |
| **Delete** / **Backspace** | Remove the selected item of the active tab |
| **Esc** | Close Script, Help, or the unsaved-changes dialog; otherwise leave the editor |

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

Click an entity type, then left-click the map to place it. Right-click removes only entities and the player start. Background and other tiles stay in place.

| Entity | Notes |
| --- | --- |
| `player` | Start position. You can place more than one. |
| `item-package` / `item-package-ice` | Packages to deliver. Ice packages belong on ice-themed maps. |
| `item-stone` | Drop on dinosaurs or trees. |
| `item-apple` / `item-banana` / `item-egg` | Pickup items (health, strength, invulnerability). |
| `item-bomb` | Explosive pickup. |
| `npc-walking` / `npc-blowing` / `npc-mammut` | Ground dinosaurs. Space flips facing. |
| `tree` | Drop a stone on it for fruit. |

**Emitter amount** and **Emitter delay** (Properties) apply to the next placed emitter: how many entities spawn and the wait in milliseconds before spawning.

Fish and pterodactyls are not placed as entities. Enable them with the **Activate the fish spawn** and **Activate the pterodactyls spawn** checkboxes so they appear according to map water and flying space.

Friendly cave NPCs (`npc-man`, `npc-woman`, `npc-grandpa`) are assigned on cave tiles in the map file, not from this list. Taxi/rescue maps also need `npctransfercount` in the Lua settings (see [Map scripts](#map-scripts-and-lua)).

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
| Activate the pterodactyls spawn | Random flying enemy. |
| Activate the fish spawn | Random fish in water. |
| Npc delay | Default cave spawn delay in milliseconds. |
| Theme | Switches the palette and remaps some existing tiles when changing theme. |
| Seed + Auto | Generate a random map for the current theme. Seed `0` picks a new seed each time. |

### Gates and pressure plates

Select a plate or gate (Tiles tab). Properties then show **Trigger link**:

- **Link id** - both sides of a pair must share the same id (for example `door1`).
- **Pick gate target** - click a plate, press the button, then click the gate.
- Plate **Required weight** (default `700`) and **Hold ms**.
- Gate **Open amount** (`0`-`1`).

A yellow line is drawn between linked partners.

## Maps tab

Lists installed and previously saved maps. Click a name to load it (you are asked to confirm if the current map is dirty). The filter box searches by filename.

## Saving and testing

| Action | Result |
| --- | --- |
| **Save** / **Ctrl+S** | Writes the Lua map file and reloads the map list. |
| **Save & Go** | Saves and starts the map immediately. |

Maps are written to the user data directory:

`~/.local/share/caveexpress/base/caveexpress/maps/<file>.lua`

(on other platforms, the equivalent SDL preference path). Built-in maps live in `base/caveexpress/maps/` next to the game data.

After saving, the map appears on the **Maps** tab and can be started with `map <file>` from the console.

If you want the map in a campaign, add its filename (without `.lua`) to a campaign script in `base/caveexpress/campaigns/`, for example:

```lua
c:addMaps("mymap")
```

## Map scripts and Lua

**Script** opens a Lua editor for logic that is kept across saves: `onMapLoaded`, `onUpdate`, and helper functions. `getName` and `initMap` are regenerated from the editor data on every save, so do not keep hand-edited tiles there if you will save from the editor again.

Useful settings that are not all in the Properties panel can be added in `initMap` after a save, or kept in mind when designing:

| Setting | Meaning |
| --- | --- |
| `packagetransfercount` | Packages required (also in Properties). |
| `npctransfercount` | Friendly NPCs to deliver to a target cave. |
| `waterchangespeed` | Rising/falling water. |
| `waterrisingdelay` / `waterfallingdelay` | Delay before water starts moving. |
| `sideborderfail` | Fail if you touch the side border (used on some wind maps). |
| `flyingnpc` / `fishnpc` | Also in Properties. |

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
