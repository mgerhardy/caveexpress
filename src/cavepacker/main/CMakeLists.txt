project(cavepacker)
set(TPS
	cavepacker-entity
	cavepacker-environment
	cavepacker-ui-fonts
	cavepacker-ui-scene
	cavepacker-ui
)
cp_add_executable(TARGET ${PROJECT_NAME} TPS ${TPS} SRCS CavePacker.cpp ../../Main.cpp WINDOWED APPNAME "CavePacker" VERSION 1.6 VERSION_CODE 11 CATEGORY LogicGame)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${PROJECT_NAME}-server ${PROJECT_NAME}-client ${PROJECT_NAME}-shared game)
set(LUAFILES
	config.lua
	fonts.lua
	sprites.lua
	textures/cavepacker-entity-big.lua
	textures/cavepacker-entity-small.lua
	textures/cavepacker-environment-big.lua
	textures/cavepacker-environment.lua
	textures/cavepacker-environment-small.lua
	textures/cavepacker-ui-big.lua
	textures/cavepacker-ui-fonts-big.lua
	textures/cavepacker-ui-fonts-small.lua
	textures/cavepacker-ui-scene-big.lua
	textures/cavepacker-ui-scene-small.lua
	textures/cavepacker-ui-small.lua
)
check_lua_files(${PROJECT_NAME} "${LUAFILES}")
