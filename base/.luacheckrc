-- Lua 5.4 (src/libs/CMakeLists.txt fetches lua 5.4.4).
-- C bindings for CaveExpress / CavePacker scripts under base/.
stds.caveexpress = {
	read_globals = {
		-- LUALibrary.cpp (every Lua state)
		isAndroid = {},
		isWindows = {},
		isMacOSX = {},
		isLinux = {},
		isHTML5 = {},
		isDebug = {},
		isHD = {},
		isTouch = {},

		-- MapScript.cpp
		isKeyPressed = {},

		-- Intro.cpp (function intro(help) in map scripts)
		tr = {},

		-- LUAMapContext.cpp, CaveExpressMapContext.h, MapScript.cpp
		Map = {
			fields = {
				get = {},
				addTile = {},
				setSetting = {},
				addEmitter = {},
				addStartPosition = {},
				addCave = {},
				addGate = {},
				addPressurePlate = {},
				finish = {},
				isDone = {},
				getTime = {},
				getSize = {},
				setInputEnabled = {},
				isInputEnabled = {},
				isKeyPressed = {},
				consumeSkip = {},
				message = {},
				getGravity = {},
				calculateVelocity = {},
				getPlayer = {},
				getEntity = {},
				getCaveCount = {},
				getCave = {},
				getWaterHeight = {},
				setWaterHeight = {},
				spawnPackage = {},
				getPackageCount = {},
				getPackage = {},
				getPackages = {},
				getPackageTarget = {},
				getDeliveredPackageCount = {},
				getCollectedPackageCount = {},
				getPackageDeliveryGoal = {},
				spawnPackageNPC = {},
				spawnFriendlyNPC = {},
				spawnNPC = {},
				addTileRuntime = {},
				removeTileAt = {},
				replaceTile = {},
				rebuildPlatforms = {},
				removeEntity = {},
			},
		},

		-- CampaignManager.cpp (base/*/campaigns/*.lua)
		Campaign = {
			fields = {
				new = {},
				addMap = {},
				addMaps = {},
				unlockMap = {},
				unlock = {},
				setSetting = {},
				loadProgress = {},
			},
		},

		-- MapScript.cpp (entity userdata from Map.getPlayer / getCave / …)
		Entity = {
			fields = {
				isValid = {},
				getId = {},
				getType = {},
				getPos = {},
				setPos = {},
				getVelocity = {},
				setVelocity = {},
				applyImpulse = {},
				applyForce = {},
				getGravityScale = {},
				setGravityScale = {},
				getGravity = {},
				getState = {},
				setState = {},
				remove = {},
				isPlayer = {},
				isNpc = {},
				isCave = {},
				accelerate = {},
				resetAcceleration = {},
				setMoving = {},
				setIdle = {},
				isIdle = {},
				setDone = {},
				returnToCave = {},
				leavePackage = {},
				dropPackage = {},
				setInvulnerable = {},
				drop = {},
				getCollectedPackageCount = {},
				getCollectedPackages = {},
				isPackage = {},
				isCollected = {},
				isArrived = {},
				isDelivered = {},
				isDestroyed = {},
				isPulling = {},
				getPullingPackage = {},
				setTargetCave = {},
				getTargetCave = {},
				setAnimation = {},
				getLightState = {},
				setLightState = {},
				setNextSpawn = {},
				setRespawnPossible = {},
				spawnCaveNPC = {},
				getCaveNumber = {},
			},
		},

		-- Intro.cpp (help argument of function intro)
		Intro = {
			fields = {
				headline = {},
				text = {},
				entity = {},
				beginRow = {},
				endRow = {},
				bar = {},
			},
		},
	},
}

std = "lua54+caveexpress"
max_line_length = false
allow_defined = true
ignore = {
	"131", -- unused global variable (sprites, settings, fonts, textures, …)
	"212", -- unused argument
	"431", -- shadowing upvalue
}
