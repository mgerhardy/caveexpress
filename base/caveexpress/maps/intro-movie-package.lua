-- Intro movie: "The garbage problem"
-- Fully scripted cutscene - no player interaction.
-- Rubbish sits in front of the cave. A villager has an idea, walks into the
-- pile, vanishes in the dust, then reappears beside the new machine and boards.
-- While the pilot sits idle, another villager leaves the cave, has an idea,
-- and hauls packed garbage out. The machine then delivers it to the target.
--
-- After the machine appears the builder stands beside it, waits, and boards.
-- Only then does the woman leave the cave. After the last delivery the machine
-- lands next to the cave, the villager walks home, and the cave light goes out.
--
-- Coordinate note: Y grows downward (gravity). "Above" means a smaller Y.

function getName()
	return "Intro Movie"
end

local phase = "boot"
local timer = 0
local pilot = nil
local player = nil
local waste = nil
local dust = nil
local idea = nil
local dumper = nil
local dumperDrops = 0
local drops = 0
local packages = {}
local flyPath = nil
local flyWaypoint = 1
local playMode = "idle"
local deliveredAtStart = 0
local deliverWait = 0
local hoverWait = 0
local settleFrames = 0
local droppedPackage = nil
local deliverAttempts = 0
local collectTarget = nil
local MSG_MS = 4500
local SETTLE_SPEED = 0.35
local HOVER_MIN_MS = 900
local HOVER_MAX_MS = 3500
local DELIVER_TIMEOUT_MS = 8000
local MAX_DELIVER_ATTEMPTS = 3
local WASTE_X = 3.0
local WASTE_Y = 3.5
local WASTE_W = 3.0
local WASTE_H = 1.5
local WASTE_CX = WASTE_X + WASTE_W * 0.5
local WASTE_APPROACH_X = 3.15
local WASTE_WORK_X = 4.5
local BOARD_WAIT_MS = 2000
local NPC_GROUND_Y = 4.726 -- ground 5.0 - npc-man height 0.548 / 2
local DUST_W = 2.0
local DUST_H = 1.4
-- Dust plays on the front layer (over the machine/NPC). Place them while the
-- puff is still up, then clear the dust so they are revealed, not spawned.
local DUST_HIDE_NPC_MS = 220
local DUST_REVEAL_MS = 420
local DUST_CLEAR_MS = 1750
-- Land far enough right of the cave that a villager can step out and walk in.
local HOME_X = 3.25
local CAVE_ENTER_X = 1.50
local DEBOARD_OFFSET = 0.95
local PLAYER_HALF_H = 0.435
-- Between the cave (x=1) and the machine, on the walkway.
local DUMP_X = { 2.15, 2.60, 3.05 }
local MACHINE_HIDE_X = -4.0
local machineShown = false

local function setPhase(map, name, message)
	phase = name
	timer = 0
	if message then
		map:message(message, MSG_MS)
	end
end

local function removeDeco(ent)
	if ent ~= nil and ent:isValid() then
		ent:remove()
	end
	return nil
end

-- Cave-sign style: 1x1 tile whose sprite sits at the bottom of the cell,
-- parked just above the entity's head.
local function ideaAbove(map, entity)
	if entity == nil or not entity:isValid() then
		return nil
	end
	local x, y = entity:getPos()
	return map:addTileRuntime("idea", x - 0.5, y - 1.32)
end

local function placeIdea(map, entity)
	idea = removeDeco(idea)
	idea = ideaAbove(map, entity)
	return idea
end

local function spawnWaste(map)
	waste = removeDeco(waste)
	waste = map:addTileRuntime("waste", WASTE_X, WASTE_Y)
end

local function spawnDust(map)
	dust = removeDeco(dust)
	-- LOWER_LEFT tiles sit on gridY+height. Put that baseline on the same
	-- ground line as the waste so the puff covers the pile instead of hanging above it.
	local wasteBottom = WASTE_Y + WASTE_H
	dust = map:addTileRuntime("dust", WASTE_CX - DUST_W * 0.5, wasteBottom - DUST_H)
end

local function rememberPackage(pkg)
	if pkg ~= nil and pkg:isValid() then
		packages[#packages + 1] = pkg
	end
end

local function refreshPackages(map)
	packages = map:getPackages() or {}
end

local function nextFreePackage(map)
	refreshPackages(map)
	for i = 1, #packages do
		local pkg = packages[i]
		if pkg ~= nil and pkg:isValid()
			and not pkg:isCollected()
			and not pkg:isDelivered()
			and not pkg:isArrived()
			and not pkg:isDestroyed() then
			return pkg
		end
	end
	return nil
end

local function ensurePackages(map)
	refreshPackages(map)
	while #packages < 3 do
		local x = 2.4 + #packages * 0.85
		rememberPackage(map:spawnPackage(x, 3.5))
	end
	drops = math.max(drops, #packages)
end

local function endScriptedFlight(playerEnt)
	if playerEnt ~= nil and playerEnt:isValid() then
		playerEnt:resetAcceleration()
		playerEnt:setGravityScale(1)
		playerEnt:setVelocity(0, 0)
		playerEnt:setAnimation("idle")
	end
	flyPath = nil
	flyWaypoint = 1
end

local function keepEmpty(playerEnt)
	if playerEnt ~= nil and playerEnt:isValid() then
		-- Parked machine: no gravity drift, fixed on the ground surface (y=5).
		local groundY = 5.0
		playerEnt:setGravityScale(0)
		playerEnt:resetAcceleration()
		playerEnt:setVelocity(0, 0)
		playerEnt:setPos(WASTE_CX, groundY - PLAYER_HALF_H)
		playerEnt:setAnimation("empty")
	end
end

local function hideMachine(playerEnt)
	if playerEnt == nil or not playerEnt:isValid() then
		return
	end
	-- Park off the left edge so the finished heli is not drawn until the reveal.
	local groundY = 5.0
	playerEnt:setGravityScale(0)
	playerEnt:resetAcceleration()
	playerEnt:setVelocity(0, 0)
	playerEnt:setPos(MACHINE_HIDE_X, groundY - PLAYER_HALF_H)
	playerEnt:setAnimation("empty")
end

local function parkMachine(playerEnt)
	if machineShown then
		keepEmpty(playerEnt)
	else
		hideMachine(playerEnt)
	end
end

local function hideNpc(ent)
	if ent == nil or not ent:isValid() then
		return
	end
	ent:setIdle()
	ent:setPos(MACHINE_HIDE_X, NPC_GROUND_Y)
end

local function placeNpc(ent, x)
	if ent == nil or not ent:isValid() then
		return
	end
	ent:setPos(x, NPC_GROUND_Y)
	ent:setIdle()
	ent:setVelocity(0, 0)
end

-- Stand flush with the left side of the parked machine (player-empty).
local function machineStandX(playerEnt)
	local px = WASTE_CX
	if playerEnt ~= nil and playerEnt:isValid() then
		local x = select(1, playerEnt:getPos())
		if x > 1.5 then
			px = x
		end
	end
	return px - 0.52
end

local function standBesideMachine(ent, playerEnt)
	placeNpc(ent, machineStandX(playerEnt))
end

-- Left of the parked machine with a gap so the bodies do not overlap.
local function deboardStandX(playerEnt)
	local px = HOME_X
	if playerEnt ~= nil and playerEnt:isValid() then
		local x = select(1, playerEnt:getPos())
		if x > 1.5 then
			px = x
		end
	end
	return px - DEBOARD_OFFSET
end

local function caveWalkX(map)
	local cave = map:getCave(1)
	if cave ~= nil then
		return select(1, cave:getPos())
	end
	return CAVE_ENTER_X
end

-- Place the parked machine and builder while dust is still covering them.
-- Dust stays; it is a front-layer sprite so it draws over both.
local function showMachineUnderDust(playerEnt)
	waste = removeDeco(waste)
	idea = removeDeco(idea)
	machineShown = true
	keepEmpty(playerEnt)
end

local function revealMachine(playerEnt)
	waste = removeDeco(waste)
	dust = removeDeco(dust)
	idea = removeDeco(idea)
	machineShown = true
	keepEmpty(playerEnt)
end

local function parkAt(playerEnt, x)
	if playerEnt == nil or not playerEnt:isValid() then
		return
	end
	playerEnt:setGravityScale(0)
	playerEnt:resetAcceleration()
	playerEnt:setVelocity(0, 0)
	playerEnt:setPos(x, 5.0 - PLAYER_HALF_H)
	playerEnt:setInvulnerable(60000)
end

-- Skip finishes the cutscene; never hands control to the player.
local function skipCutscene(map)
	map:consumeSkip()
	revealMachine(player or map:getPlayer())
	ensurePackages(map)
	player = player or map:getPlayer()
	if player ~= nil and player:isValid() then
		endScriptedFlight(player)
		keepEmpty(player)
	end
	if pilot ~= nil and pilot:isValid() then
		pilot:remove()
	end
	pilot = nil
	if dumper ~= nil and dumper:isValid() then
		dumper:remove()
	end
	dumper = nil
	map:setInputEnabled(false)
	setPhase(map, "done", "The caves are clean again. Thank you!")
	map:finish()
end

local function flyToward(entity, tx, ty, speed)
	local x, y = entity:getPos()
	local dx, dy = tx - x, ty - y
	local dist = math.sqrt(dx * dx + dy * dy)
	if dist < 0.3 then
		entity:setVelocity(0, 0)
		entity:setAnimation("flying")
		return true
	end
	entity:setGravityScale(0)
	entity:resetAcceleration()
	entity:setVelocity(dx / dist * speed, dy / dist * speed)
	-- Apply after resetAcceleration(), which would otherwise force idle.
	entity:setAnimation("flying")
	return false
end

local function buildCollectPath(playerEnt, pkg)
	local px, py = playerEnt:getPos()
	local cx, cy = pkg:getPos()
	local cruiseY = math.min(py, cy) - 1.0
	if cruiseY < 0.8 then
		cruiseY = 0.8
	end
	local landY = cy - 0.25
	return {
		{ px, cruiseY, 2.2 },
		{ cx, cruiseY, 2.6 },
		{ cx, landY, 1.4 },
	}
end

local function buildHomePath(playerEnt)
	local px, py = playerEnt:getPos()
	local landY = 5.0 - PLAYER_HALF_H
	local cruiseY = math.min(py, landY) - 2.0
	if cruiseY < 0.6 then
		cruiseY = 0.6
	end
	return {
		{ px, cruiseY, 2.0 },
		{ HOME_X, cruiseY, 2.2 },
		{ HOME_X, landY - 0.40, 0.85 },
		{ HOME_X, landY, 0.40 },
	}
end

local function beginHome(map, playerEnt)
	if playerEnt == nil or not playerEnt:isValid() then
		setPhase(map, "done")
		map:finish()
		return
	end
	playMode = "flyHome"
	flyWaypoint = 1
	flyPath = buildHomePath(playerEnt)
	playerEnt:setInvulnerable(60000)
	playerEnt:setGravityScale(0)
	setPhase(map, "home", "The caves are clean again. Thank you!")
end

local function buildDeliverPath(playerEnt, tx, ty)
	local px, py = playerEnt:getPos()
	local cruiseY = math.min(py, ty) - 2.2
	if cruiseY < 0.6 then
		cruiseY = 0.6
	end
	local hoverY = ty - 1.35
	if hoverY < cruiseY then
		hoverY = cruiseY
	end
	return {
		{ px, cruiseY, 2.0 },
		{ tx, cruiseY, 2.4 },
		{ tx, hoverY, 1.2 },
	}
end

local function packageSpeed(pkg)
	if pkg == nil or not pkg:isValid() then
		return 0
	end
	local vx, vy = pkg:getVelocity()
	return math.sqrt(vx * vx + vy * vy)
end

local function firstCarriedPackage(playerEnt)
	local carried = playerEnt:getCollectedPackages()
	if carried == nil or #carried < 1 then
		return nil
	end
	return carried[1]
end

local function packageHitTarget(pkg, target)
	if pkg == nil or not pkg:isValid() then
		return false
	end
	if pkg:isArrived() or pkg:isDelivered() then
		return true
	end
	if target ~= nil and target:isValid() and target:isPulling() then
		local pulling = target:getPullingPackage()
		if pulling ~= nil and pulling:isValid() and pulling:getId() == pkg:getId() then
			return true
		end
	end
	return false
end

local function beginCollect(map, playerEnt, pkg)
	playMode = "collect"
	collectTarget = pkg
	flyWaypoint = 1
	flyPath = buildCollectPath(playerEnt, pkg)
	playerEnt:setInvulnerable(60000)
end

local function beginDeliver(map, playerEnt)
	playMode = "deliver"
	deliveredAtStart = map:getDeliveredPackageCount()
	deliverWait = 0
	hoverWait = 0
	settleFrames = 0
	droppedPackage = firstCarriedPackage(playerEnt)
	playerEnt:setInvulnerable(60000)
	flyWaypoint = 1
	local target = map:getPackageTarget()
	if target ~= nil and target:isValid() then
		local tx, ty = target:getPos()
		flyPath = buildDeliverPath(playerEnt, tx, ty)
		return true
	end
	flyPath = nil
	playMode = "idle"
	return false
end

local function npcCloseTo(ent, x, maxDist)
	if ent == nil or not ent:isValid() then
		return false
	end
	return math.abs(select(1, ent:getPos()) - x) < (maxDist or 0.35)
end

local function beginDumper(map)
	dumper = nil
	dumperDrops = 0
	idea = removeDeco(idea)
	setPhase(map, "dumperOut", "Someone else has an idea...")
end

local function beginRescueFlight(map, playerEnt)
	refreshPackages(map)
	ensurePackages(map)
	local pkg = nextFreePackage(map)
	if pkg == nil then
		beginHome(map, playerEnt)
		return
	end
	setPhase(map, "rescue", "CaveExpress to the rescue!")
	playMode = "collect"
	beginCollect(map, playerEnt, pkg)
end

function onMapLoaded()
	local map = Map.get()
	map:setInputEnabled(false)
	packages = {}
	flyPath = nil
	flyWaypoint = 1
	playMode = "idle"
	deliveredAtStart = 0
	deliverWait = 0
	hoverWait = 0
	settleFrames = 0
	droppedPackage = nil
	deliverAttempts = 0
	collectTarget = nil
	pilot = nil
	dumper = nil
	dumperDrops = 0
	idea = nil
	dust = nil
	waste = nil
	machineShown = false
	local cave = map:getCave(1)
	if cave ~= nil then
		cave:setRespawnPossible(false)
		cave:setLightState(true)
	end
	player = map:getPlayer()
	if player ~= nil and player:isValid() then
		player:setInvulnerable(60000)
		parkMachine(player)
	end
	spawnWaste(map)
	setPhase(map, "boot", "Long ago in the caves...")
end

function onUpdate(dt)
	local map = Map.get()
	timer = timer + dt

	if phase ~= "done" and isKeyPressed("skip") and timer > 600 then
		skipCutscene(map)
		return
	end

	player = player or map:getPlayer()

	if phase == "boot" then
		parkMachine(player)
		if timer > 3500 then
			setPhase(map, "appear", "Others left their rubbish in front of the caves.")
		end
		return
	end

	-- Villager leaves the cave and walks to the waste. No idea bubble yet.
	if phase == "appear" then
		parkMachine(player)
		if pilot == nil then
			pilot = map:spawnFriendlyNPC(1, "npc-man", false)
			if pilot == nil or not pilot:isValid() then
				revealMachine(player)
				ensurePackages(map)
				if player ~= nil and player:isValid() then
					player:setAnimation("idle")
				end
				beginRescueFlight(map, player)
				return
			end
			pilot:setIdle()
		end
		if timer > 1800 then
			if pilot ~= nil and pilot:isValid() then
				pilot:setMoving(WASTE_APPROACH_X)
			end
			setPhase(map, "walkToWaste")
		end
		return
	end

	if phase == "walkToWaste" then
		parkMachine(player)
		if pilot == nil or not pilot:isValid() then
			revealMachine(player)
			ensurePackages(map)
			if player ~= nil and player:isValid() then
				player:setAnimation("idle")
			end
			beginRescueFlight(map, player)
			return
		end
		local nx = select(1, pilot:getPos())
		if timer > 400 then
			pilot:setMoving(WASTE_APPROACH_X)
		end
		if math.abs(nx - WASTE_APPROACH_X) < 0.35 or timer > 12000 then
			pilot:setIdle()
			placeIdea(map, pilot)
			setPhase(map, "think", "We could build something from this!")
		end
		return
	end

	if phase == "think" then
		parkMachine(player)
		if pilot ~= nil and pilot:isValid() then
			pilot:setIdle()
		end
		if timer > 2500 then
			idea = removeDeco(idea)
			if pilot ~= nil and pilot:isValid() then
				pilot:setMoving(WASTE_WORK_X)
			end
			setPhase(map, "walkIntoWaste")
		end
		return
	end

	-- Walk into the pile, then vanish inside the dust cloud.
	if phase == "walkIntoWaste" then
		parkMachine(player)
		if pilot == nil or not pilot:isValid() then
			spawnDust(map)
			setPhase(map, "dust")
			return
		end
		local nx = select(1, pilot:getPos())
		if timer > 400 then
			pilot:setMoving(WASTE_WORK_X)
		end
		if math.abs(nx - WASTE_WORK_X) < 0.35 or timer > 8000 then
			spawnDust(map)
			setPhase(map, "dust")
		end
		return
	end

	-- Dust covers the pile. The machine and builder are placed underneath while
	-- the puff is still playing (front layer), then the dust is removed.
	if phase == "dust" then
		if timer > DUST_HIDE_NPC_MS then
			hideNpc(pilot)
		end
		if timer > DUST_REVEAL_MS then
			showMachineUnderDust(player)
			standBesideMachine(pilot, player)
		else
			parkMachine(player)
		end
		if timer > DUST_CLEAR_MS then
			dust = removeDeco(dust)
			setPhase(map, "reveal", "The CaveExpress is ready!")
		end
		return
	end

	if phase == "reveal" then
		keepEmpty(player)
		-- Pin beside the machine; do not start a walk.
		standBesideMachine(pilot, player)
		if timer > BOARD_WAIT_MS then
			setPhase(map, "board")
		end
		return
	end

	if phase == "pilotWalk" then
		-- Legacy phase: stand and board, do not walk in from far away.
		keepEmpty(player)
		standBesideMachine(pilot, player)
		if timer > BOARD_WAIT_MS or pilot == nil or not pilot:isValid() then
			setPhase(map, "board")
		end
		return
	end

	if phase == "board" then
		if pilot ~= nil and pilot:isValid() then
			pilot:remove()
		end
		pilot = nil
		if player ~= nil and player:isValid() then
			player:setAnimation("idle")
			player:setInvulnerable(60000)
		end
		if timer > 400 then
			beginDumper(map)
		end
		return
	end

	-- Second villager leaves the cave, has an idea, and dumps packed garbage.
	if phase == "dumperOut" then
		keepEmpty(player)
		if dumper == nil then
			dumper = map:spawnPackageNPC(1, "npc-woman")
			if dumper == nil or not dumper:isValid() then
				ensurePackages(map)
				beginRescueFlight(map, player)
				return
			end
			dumper:setMoving(DUMP_X[1])
		elseif dumper:isValid() and not npcCloseTo(dumper, DUMP_X[1], 0.35) and timer < 10000 then
			if timer > 400 and math.floor(timer / 450) ~= math.floor((timer - dt) / 450) then
				dumper:setMoving(DUMP_X[1])
			end
		end
		if dumper ~= nil and dumper:isValid()
			and (npcCloseTo(dumper, DUMP_X[1], 0.35) or timer > 10000) then
			dumper:setIdle()
			dumper:setVelocity(0, 0)
			placeIdea(map, dumper)
			setPhase(map, "dumperThink", "We could pack this and fly it away!")
		end
		return
	end

	if phase == "dumperThink" then
		keepEmpty(player)
		if dumper ~= nil and dumper:isValid() then
			dumper:setIdle()
			dumper:setVelocity(0, 0)
		end
		if timer > 2400 then
			idea = removeDeco(idea)
			dumperDrops = 0
			setPhase(map, "dumperDump")
		end
		return
	end

	if phase == "dumperDump" then
		keepEmpty(player)
		if dumper == nil or not dumper:isValid() then
			ensurePackages(map)
			beginRescueFlight(map, player)
			return
		end
		local targetX = DUMP_X[math.min(dumperDrops + 1, #DUMP_X)]
		if timer > 300 then
			dumper:setMoving(targetX)
		end
		if npcCloseTo(dumper, targetX, 0.4) or timer > 8000 then
			dumper:setIdle()
			local pkg = dumper:dropPackage()
			rememberPackage(pkg)
			dumperDrops = dumperDrops + 1
			timer = 0
			if dumperDrops >= #DUMP_X then
				dumper:returnToCave()
				setPhase(map, "dumperHome")
			end
		end
		return
	end

	if phase == "dumperHome" then
		keepEmpty(player)
		if dumper == nil or not dumper:isValid() or timer > 10000 then
			dumper = nil
			ensurePackages(map)
			beginRescueFlight(map, player)
			return
		end
		if timer > 400 then
			dumper:returnToCave()
			timer = 0
		end
		return
	end

	if phase == "rescue" then
		if player == nil or not player:isValid() then
			return
		end
		player:setInvulnerable(60000)

		if playMode == "collect" then
			if player:getCollectedPackageCount() > 0 then
				deliverAttempts = 1
				beginDeliver(map, player)
				return
			end
			if flyPath == nil then
				local pkg = collectTarget
				if pkg == nil or not pkg:isValid() then
					pkg = nextFreePackage(map)
				end
				if pkg == nil then
					beginHome(map, player)
					return
				end
				beginCollect(map, player, pkg)
				return
			end
			local wp = flyPath[flyWaypoint]
			if wp == nil then
				player:setGravityScale(0.4)
				player:resetAcceleration()
				player:setVelocity(0, 0.6)
				player:setAnimation("flying")
				if timer > 1500 then
					local pkg = nextFreePackage(map)
					if pkg ~= nil then
						beginCollect(map, player, pkg)
						timer = 0
					end
				end
				return
			end
			if flyToward(player, wp[1], wp[2], wp[3]) then
				flyWaypoint = flyWaypoint + 1
				timer = 0
			end
		elseif playMode == "deliver" then
			if flyPath == nil then
				playMode = "collect"
				return
			end
			local wp = flyPath[flyWaypoint]
			if wp == nil then
				player:setGravityScale(0)
				player:resetAcceleration()
				player:setVelocity(0, 0)
				player:setAnimation("flying")
				hoverWait = hoverWait + dt
				local pkg = droppedPackage
				if pkg == nil or not pkg:isValid() or not pkg:isCollected() then
					pkg = firstCarriedPackage(player)
					droppedPackage = pkg
				end
				local speed = packageSpeed(pkg)
				if speed < SETTLE_SPEED then
					settleFrames = settleFrames + 1
				else
					settleFrames = 0
				end
				local settled = settleFrames > 12 and hoverWait >= HOVER_MIN_MS
				if settled or hoverWait >= HOVER_MAX_MS then
					player:drop()
					deliverWait = 0
					playMode = "waitDeliver"
				end
				return
			end
			if flyToward(player, wp[1], wp[2], wp[3]) then
				flyWaypoint = flyWaypoint + 1
				if flyPath[flyWaypoint] == nil then
					hoverWait = 0
					settleFrames = 0
				end
			end
		elseif playMode == "waitDeliver" then
			player:setGravityScale(0)
			player:resetAcceleration()
			player:setVelocity(0, 0)
			deliverWait = deliverWait + dt
			local target = map:getPackageTarget()
			local hit = map:getDeliveredPackageCount() > deliveredAtStart
				or packageHitTarget(droppedPackage, target)
			if hit then
				droppedPackage = nil
				deliverAttempts = 0
				local pkg = nextFreePackage(map)
				if pkg ~= nil then
					map:message("Next package...", MSG_MS)
					beginCollect(map, player, pkg)
				else
					beginHome(map, player)
				end
			elseif deliverWait > DELIVER_TIMEOUT_MS then
				if player:getCollectedPackageCount() > 0 and deliverAttempts < MAX_DELIVER_ATTEMPTS then
					deliverAttempts = deliverAttempts + 1
					beginDeliver(map, player)
				elseif droppedPackage ~= nil and droppedPackage:isValid()
					and not droppedPackage:isDelivered() and not droppedPackage:isArrived()
					and deliverAttempts < MAX_DELIVER_ATTEMPTS then
					deliverAttempts = deliverAttempts + 1
					beginCollect(map, player, droppedPackage)
				else
					local pkg = nextFreePackage(map)
					if pkg ~= nil then
						beginCollect(map, player, pkg)
					else
						beginHome(map, player)
					end
				end
			end
		end
		return
	end

	-- Fly back, land softly next to the cave, villager steps out and goes home.
	if phase == "home" then
		if player == nil or not player:isValid() then
			setPhase(map, "done")
			map:finish()
			return
		end
		player:setInvulnerable(60000)

		if playMode == "flyHome" then
			if flyPath == nil then
				flyPath = buildHomePath(player)
				flyWaypoint = 1
			end
			local wp = flyPath[flyWaypoint]
			if wp == nil then
				parkAt(player, HOME_X)
				player:setAnimation("idle")
				playMode = "deboard"
				timer = 0
				return
			end
			if flyToward(player, wp[1], wp[2], wp[3]) then
				flyWaypoint = flyWaypoint + 1
			end
			return
		end

		if playMode == "deboard" then
			parkAt(player, HOME_X)
			player:setAnimation("empty")
			if timer > 500 and (pilot == nil or not pilot:isValid()) then
				pilot = map:spawnFriendlyNPC(1, "npc-man", false)
				if pilot ~= nil and pilot:isValid() then
					placeNpc(pilot, deboardStandX(player))
				end
			elseif pilot ~= nil and pilot:isValid() then
				placeNpc(pilot, deboardStandX(player))
			end
			if timer > 1600 then
				if pilot ~= nil and pilot:isValid() then
					pilot:setMoving(caveWalkX(map))
				end
				playMode = "walkIn"
				timer = 0
			end
			return
		end

		if playMode == "walkIn" then
			parkAt(player, HOME_X)
			player:setAnimation("empty")
			local enterX = caveWalkX(map)
			if pilot == nil or not pilot:isValid() then
				local cave = map:getCave(1)
				if cave ~= nil then
					cave:setLightState(false)
				end
				playMode = "lightsOut"
				timer = 0
				return
			end
			if npcCloseTo(pilot, enterX, 0.22) or timer > 10000 then
				pilot:setIdle()
				pilot:setVelocity(0, 0)
				pilot:remove()
				pilot = nil
				local cave = map:getCave(1)
				if cave ~= nil then
					cave:setLightState(false)
				end
				playMode = "lightsOut"
				timer = 0
				return
			end
			if timer > 200 then
				pilot:setMoving(enterX)
			end
			return
		end

		if playMode == "lightsOut" then
			parkAt(player, HOME_X)
			player:setAnimation("empty")
			if timer > 2000 then
				setPhase(map, "done")
				map:finish()
			end
			return
		end
		return
	end
end

function initMap()
	local map = Map.get()

	for x = 0, 13 do
		for y = 0, 7 do
			map:addTile("tile-background-0" .. tostring((x + y) % 4 + 1), x, y)
		end
	end

	for x = 0, 13 do
		map:addTile("tile-ground-04", x, 5)
		map:addTile("tile-rock-02", x, 6)
		map:addTile("tile-rock-03", x, 7)
	end

	map:addTile("tile-rock-02", 0, 2)
	map:addTile("tile-rock-02", 0, 3)
	map:addTile("tile-rock-02", 0, 4)
	map:addCave("tile-cave-01", 1, 4, "npc-man", 60000)

	map:addTile("tile-rock-02", 10, 4)
	map:addTile("tile-packagetarget-rock-01-idle", 11, 4)
	map:addTile("tile-rock-02", 12, 4)
	map:addTile("tile-rock-03", 13, 2)
	map:addTile("tile-rock-03", 13, 3)
	map:addTile("tile-rock-03", 13, 4)

	-- Start off-map so the finished heli never flashes before onMapLoaded hides it.
	-- Reveal teleports to the waste pile (x=4.5, y=4.33 on the floor).
	map:addStartPosition("-4", "4.33")

	map:setSetting("width", "14")
	map:setSetting("height", "8")
	map:setSetting("theme", "rock")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "90")
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
	map:setSetting("cutscene", "true")
	map:setSetting("introwindow", "")
end
