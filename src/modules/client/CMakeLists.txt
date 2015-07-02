set(client_SRCS
	ClientConsole.cpp
	Camera.cpp
	ClientMap.cpp
	entities/ClientEntity.cpp
	entities/ClientPlayer.cpp
	entities/ClientMapTile.cpp
)
add_library(client ${client_SRCS})
set_target_properties(client PROPERTIES FOLDER client)
cp_target_link_libraries(TARGET client LIBS gfx particle service)
