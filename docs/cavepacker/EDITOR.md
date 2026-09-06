# CavePacker map editor

This guide covers the built-in Sokoban editor (`UIMapEditorWindow`, `MapEditorDocument`).

## Opening the editor

- Main menu or `cavepacker -ui_push editor`
- Console (`Shift+Tab`): `ui_push editor`

Maps are [Sokoban](../../base/cavepacker/maps/README.mapformat) `.sok` files. **Save** writes to the user maps folder. **Save to game data** writes `base/cavepacker/maps/<file>.sok`. The title is stored as a `Title:` header line.

Unsaved changes are confirmed before leaving the editor, starting a new map, or loading another map.

## Window layout

The same docked ImGui layout as CaveExpress: palette on the left, map in the center, Properties and Layers on the right. Drag title bars to undock or resize.

**Tiles** and **Entities** are separate edit modes. Floors, walls, and targets are tiles. The player start and packages are entities. A cell can hold a floor or a target, plus one package.

## Palette

**Tiles:** floors (`tile-background-*`), walls (`tile-rock-*`), and targets. Right-click for shape editing, `sprites.lua`, remove-all, and jump-to-first.

**Entities:** the player start and packages. Packages sit on floors or targets. Walls replace everything in a cell.

## Sokoban rules

| Character | Meaning |
| --- | --- |
| `#` | Wall |
| `@` | Player |
| `$` | Package |
| `.` | Target |
| `*` | Package on target |
| `+` | Player on target |
| space | Floor |

The editor enforces:

- Packages cannot be placed on walls.
- The player start cannot be placed on a wall.
- Floors and targets cannot paint over a wall (use erase or paint a wall to replace the cell).
- Flood fill does not punch floors through walls.
- **Package count must match target count** (checked on save).

The selected cell's Sokoban character is shown on Properties.

## Properties

- **Make playable** adds a wall border, interior floors, one package, one target, and a start if they are missing.
- **Auto-tile walls** picks `tile-rock-*` art from neighboring floors (the same placement rules as map load). Existing art is kept when it still matches. Paint and erase update neighbors the same way; the button records undo.
- **Check layout** flood-fills from the player along playable cells. Packages and targets must be reachable.
- Start positions can be edited, deleted, or used as **Play**.
- Campaign add/remove/create works on `base/cavepacker/campaigns/*.lua` (`c:addMaps`, including wildcards such as `tutorial*`). Wildcard membership is listed but not removed from here.

## Tools

Same mouse and key bindings as the CaveExpress editor (see [../caveexpress/EDITOR.md](../caveexpress/EDITOR.md)): paint, erase, select, fill, copy/paste, nudge, undo, and **Save & Go**.

## Shapes

**Shapes** edits collision polygons in `sprites.lua` the same way as CaveExpress. See [../caveexpress/SPRITES.md](../caveexpress/SPRITES.md).
