set(caveexpress-server_SRCS
	map/RandomMapContext.cpp
	map/Map.cpp
	events/GameEventHandler.cpp
	entities/CollectableEntity.cpp
	entities/Stone.cpp
	entities/Platform.cpp
	entities/npcs/INPCCave.cpp
	entities/npcs/NPCBlowing.cpp
	entities/npcs/NPC.cpp
	entities/npcs/NPCAttacking.cpp
	entities/npcs/NPCPackage.cpp
	entities/npcs/NPCFish.cpp
	entities/npcs/NPCFlying.cpp
	entities/npcs/NPCFriendly.cpp
	entities/npcs/NPCAggressive.cpp
	entities/Package.cpp
	entities/Egg.cpp
	entities/MapTile.cpp
	entities/WorldParticle.cpp
	entities/modificators/WindModificator.cpp
	entities/EntityEmitter.cpp
	entities/Tree.cpp
	entities/IEntity.cpp
	entities/Geyser.cpp
	entities/Water.cpp
	entities/PackageTarget.cpp
	entities/CaveMapTile.cpp
	entities/Bomb.cpp
	entities/Player.cpp
	entities/Border.cpp
	entities/Fruit.cpp
	entities/WindowTile.cpp
)
add_library(caveexpress-server ${caveexpress-server_SRCS})
set_target_properties(caveexpress-server PROPERTIES FOLDER caveexpress-server)
cp_target_link_libraries(TARGET caveexpress-server LIBS server caveexpress-shared)
