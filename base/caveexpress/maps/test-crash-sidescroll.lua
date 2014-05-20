function getName()
	return ""
end

function onMapLoaded()
end

function initMap()
	-- get the current map context
	local map = Map.get()
	map:addTile("tile-packagetarget-ice-01-idle", 0, 0, 90)
	map:addTile("tile-background-ice-cave-art-01", 0, 1)
	map:addTile("tile-ground-ice-big-01", 0, 2)
	map:addTile("tile-background-ice-05", 1, 0)
	map:addTile("tile-background-ice-05", 1, 1)
	map:addTile("tile-background-ice-04", 2, 0)
	map:addTile("tile-background-ice-01", 2, 1)
	map:addTile("tile-ground-ice-big-01", 2, 2)
	map:addTile("tile-background-ice-cave-art-01", 3, 0)
	map:addTile("tile-background-ice-08", 3, 1)
	map:addTile("tile-background-ice-02", 4, 0)
	map:addTile("tile-background-ice-05", 4, 1)
	map:addTile("tile-ground-ice-big-01", 4, 2)
	map:addTile("tile-background-ice-cave-art-02", 5, 0)
	map:addTile("tile-background-ice-05", 5, 1)

	map:addEmitter("item-package-ice", 0, 1, 1, 0, "")

	map:setSetting("width", "6")
	map:setSetting("height", "4")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "false")
	map:setSetting("gravity", "9.11")
	map:setSetting("initialspawntime", "0")
	map:setSetting("packagetransfercount", "3")
	map:setSetting("playerX", "3")
	map:setSetting("playerY", "1")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "30")
	map:setSetting("sideborderfail", "true")
	map:setSetting("theme", "ice")
	map:setSetting("waterchangespeed", "0")
	map:setSetting("waterfallingdelay", "0")
	map:setSetting("waterheight", "1")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("wind", "6")
end
