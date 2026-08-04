function getName()
	return "Gate plate tutorial"
end

function onMapLoaded()
end

function initMap()
	local map = Map.get()
	map:addTile("tile-background-01", 0, 0)
	map:addTile("tile-background-01", 1, 0)
	map:addTile("tile-background-01", 2, 0)
	map:addTile("tile-background-01", 3, 0)
	map:addTile("tile-background-01", 4, 0)
	map:addTile("tile-background-01", 5, 0)
	map:addTile("tile-background-01", 6, 0)
	map:addTile("tile-background-01", 7, 0)

	map:addTile("tile-background-02", 0, 1)
	map:addTile("tile-background-02", 1, 1)
	map:addTile("tile-background-02", 2, 1)
	map:addTile("tile-background-02", 3, 1)
	map:addTile("tile-background-02", 4, 1)
	map:addTile("tile-background-02", 5, 1)
	map:addTile("tile-background-02", 6, 1)
	map:addTile("tile-background-02", 7, 1)

	map:addTile("tile-background-03", 0, 2)
	map:addTile("tile-background-03", 1, 2)
	map:addTile("tile-background-03", 2, 2)
	map:addTile("tile-background-03", 3, 2)
	map:addTile("tile-background-03", 4, 2)
	map:addTile("tile-background-03", 5, 2)
	map:addTile("tile-background-03", 6, 2)
	map:addTile("tile-background-03", 7, 2)

	map:addTile("tile-background-04", 0, 3)
	map:addTile("tile-background-04", 1, 3)
	map:addTile("tile-background-04", 2, 3)
	map:addTile("tile-background-04", 3, 3)
	map:addTile("tile-background-04", 4, 3)
	map:addTile("tile-background-04", 5, 3)
	map:addTile("tile-background-04", 6, 3)
	map:addTile("tile-background-04", 7, 3)

	map:addTile("tile-ground-01", 0, 4)
	map:addTile("tile-ground-01", 1, 4)
	map:addTile("tile-ground-01", 3, 4)
	map:addTile("tile-ground-01", 4, 4)
	map:addTile("tile-ground-01", 5, 4)
	map:addTile("tile-ground-01", 6, 4)
	map:addTile("tile-ground-01", 7, 4)

	map:addTile("tile-rock-01", 0, 5)
	map:addTile("tile-rock-01", 1, 5)
	map:addTile("tile-rock-01", 2, 5)
	map:addTile("tile-rock-01", 3, 5)
	map:addTile("tile-rock-01", 4, 5)
	map:addTile("tile-rock-01", 5, 5)
	map:addTile("tile-rock-01", 6, 5)
	map:addTile("tile-rock-01", 7, 5)

	-- Plate at x=2; gate blocks the corridor at x=5.
	-- Stone sits beside the plate so the gate starts closed and visible.
	map:addPressurePlate("tile-plate-01-idle", 2, 4, "door1", 700, 0)
	map:addGate("tile-gate-rock-01", 5, 3, "door1", 1.0)

	map:addEmitter("item-stone", 0, 3, 1, 0, "")

	map:addStartPosition(1, 2)
	map:addCave("tile-cave-01", 6, 3, "npc-man", 5000)

	map:setSetting("width", "8")
	map:setSetting("height", "6")
	map:setSetting("theme", "rock")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "60")
	map:setSetting("packagetransfercount", "0")
	map:setSetting("npctransfercount", "1")
	map:setSetting("waterheight", "0")
	map:setSetting("gravity", "9.81")
	map:setSetting("wind", "0.0")
	map:setSetting("flyingnpc", "false")
	map:setSetting("fishnpc", "false")
end
