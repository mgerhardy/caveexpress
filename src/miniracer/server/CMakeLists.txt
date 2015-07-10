set(miniracer-server_SRCS
	map/MiniRacerMapContext.cpp
	map/Map.cpp
	entities/MapTile.cpp
	entities/IEntity.cpp
	entities/Player.cpp
)
add_library(miniracer-server ${miniracer-server_SRCS})
set_target_properties(miniracer-server PROPERTIES FOLDER miniracer-server)
cp_target_link_libraries(TARGET miniracer-server LIBS server miniracer-shared)
