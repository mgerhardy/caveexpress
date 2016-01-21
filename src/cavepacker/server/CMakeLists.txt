set(cavepacker-server_SRCS
	map/SokobanMapContext.cpp
	map/Map.cpp
	entities/MapTile.cpp
	entities/IEntity.cpp
	entities/Player.cpp
)
add_library(cavepacker-server ${cavepacker-server_SRCS})
set_target_properties(cavepacker-server PROPERTIES FOLDER cavepacker-server)
cp_target_link_libraries(TARGET cavepacker-server LIBS mainloop cavepacker-shared)
