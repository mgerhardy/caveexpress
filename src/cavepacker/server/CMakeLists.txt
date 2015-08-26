set(cavepacker-server_SRCS
	map/SokobanMapContext.cpp
	map/Map.cpp
	map/BoardState.cpp
	map/deadlock/DeadlockDetector.cpp
	map/deadlock/SimpleDeadlockDetector.cpp
	map/deadlock/FrozenDeadlockDetector.cpp
	entities/MapTile.cpp
	entities/IEntity.cpp
	entities/Player.cpp
)
add_library(cavepacker-server ${cavepacker-server_SRCS})
set_target_properties(cavepacker-server PROPERTIES FOLDER cavepacker-server)
cp_target_link_libraries(TARGET cavepacker-server LIBS server cavepacker-shared)
