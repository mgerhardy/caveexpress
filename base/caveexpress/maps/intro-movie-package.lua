-- Intro movie: "The garbage problem"
-- Shows a cave NPC dumping packages, getting annoyed, then the player cleaning up.
-- Demonstrates map script APIs: onUpdate, spawn, entity control, input lock, keys, finish.
--
-- Controls during the interactive phase: fly with arrows/WASD, drop with Space.
-- Press Space/Enter/Escape during the cinematic to skip ahead.

function getName()
	return "Intro Movie"
end

local phase = "boot"
local timer = 0
local npc = nil
local player = nil
local drops = 0
local flyTime = 0

local function setPhase(map, name, message)
	phase = name
	timer = 0
	if message then
		map:message(message)
	end
end

function onMapLoaded()
	local map = Map.get()
	map:setInputEnabled(false)
	setPhase(map, "boot", "Long ago in the caves...")
end

function onUpdate(dt)
	local map = Map.get()
	timer = timer + dt

	-- Allow skipping cinematic beats
	if phase ~= "play" and phase ~= "done" and isKeyPressed("skip") and timer > 400 then
		if phase == "boot" or phase == "spawn" or phase == "drop" or phase == "dropwait" or phase == "annoyed" then
			setPhase(map, "handover", "Your turn! Deliver the packages.")
			map:setInputEnabled(true)
			-- Ensure packages exist if we skipped early
			if drops < 1 then
				map:spawnPackage(3.5, 3.0)
				map:spawnPackage(4.5, 3.0)
				map:spawnPackage(5.5, 3.0)
				drops = 3
			end
			return
		end
	end

	if phase == "boot" then
		if timer > 1200 then
			setPhase(map, "spawn")
		end
		return
	end

	if phase == "spawn" then
		npc = map:spawnPackageNPC(1, "npc-man")
		if npc == nil then
			-- Fallback: spawn packages directly if NPC cannot leave the cave yet
			map:spawnPackage(3.5, 3.0)
			map:spawnPackage(4.5, 3.0)
			map:spawnPackage(5.5, 3.0)
			drops = 3
			setPhase(map, "annoyed", "@#$%! Look at this mess!")
			return
		end
		setPhase(map, "drop", "Someone is dumping garbage...")
		return
	end

	if phase == "drop" then
		if npc ~= nil and drops < 3 and timer > 700 then
			npc:leavePackage()
			drops = drops + 1
			timer = 0
			if drops >= 3 then
				setPhase(map, "annoyed", "@#$%! Too much garbage!")
			end
		end
		return
	end

	if phase == "annoyed" then
		if timer > 1500 then
			setPhase(map, "flyin", "CaveExpress to the rescue!")
			player = map:getPlayer()
			flyTime = 0
		end
		return
	end

	if phase == "flyin" then
		player = player or map:getPlayer()
		if player ~= nil then
			-- Scripted approach flight toward the packages
			flyTime = flyTime + dt
			if flyTime < 900 then
				player:accelerate("down")
			elseif flyTime < 1800 then
				player:accelerate("left")
			elseif flyTime < 2600 then
				player:accelerate("down")
			else
				setPhase(map, "handover", "Land on packages, then drop them into the shredder. (Space to drop)")
				map:setInputEnabled(true)
			end
		elseif timer > 500 then
			-- Player not spawned yet - wait
			timer = 0
		end
		return
	end

	if phase == "handover" or phase == "play" then
		phase = "play"
		-- Win either by delivering packages normally, or finish after a long timeout / key
		if map:isDone() then
			setPhase(map, "done", "The caves are clean again. Thank you!")
			map:finish()
		elseif timer > 90000 or (timer > 2000 and isKeyPressed("skip")) then
			setPhase(map, "done", "Demo complete.")
			map:finish()
		end
		return
	end
end

function initMap()
	local map = Map.get()

	-- Tiny rock stage: cave on the left, shredder on the right, open flight space.
	for x = 0, 11 do
		for y = 0, 7 do
			map:addTile("tile-background-0" .. tostring((x + y) % 4 + 1), x, y)
		end
	end

	-- Ground / rock borders
	for x = 0, 11 do
		map:addTile("tile-ground-04", x, 5)
		map:addTile("tile-rock-02", x, 6)
		map:addTile("tile-rock-03", x, 7)
	end

	-- Left wall / cave pocket
	map:addTile("tile-rock-big-01", 0, 3)
	map:addTile("tile-rock-02", 0, 2)
	map:addCave("tile-cave-01", 1, 4, "npc-man", 60000)
	map:addTile("tile-rock-02", 2, 4)

	-- Right shredder platform
	map:addTile("tile-packagetarget-rock-01-idle", 9, 4)
	map:addTile("tile-rock-02", 8, 4)
	map:addTile("tile-rock-02", 10, 4)
	map:addTile("tile-rock-03", 11, 2)
	map:addTile("tile-rock-03", 11, 3)
	map:addTile("tile-rock-03", 11, 4)

	map:addStartPosition("7", "2")

	map:setSetting("width", "12")
	map:setSetting("height", "8")
	map:setSetting("theme", "rock")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "60")
	map:setSetting("packagetransfercount", "3")
	map:setSetting("npctransfercount", "0")
	map:setSetting("npcs", "0")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "false")
	map:setSetting("gravity", "9.81")
	map:setSetting("wind", "0")
	map:setSetting("waterheight", "0")
	map:setSetting("waterchangespeed", "0")
	map:setSetting("waterfallingdelay", "0")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("sideborderfail", "false")
	map:setSetting("tutorial", "true")
	map:setSetting("introwindow", "")
end
