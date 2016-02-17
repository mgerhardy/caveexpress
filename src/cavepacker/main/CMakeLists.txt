project(cavepacker)
set(TPS
	cavepacker-entity
	cavepacker-environment
	cavepacker-ui-fonts
	cavepacker-ui-scene
	cavepacker-ui
)
cp_add_executable(TARGET ${PROJECT_NAME} TPS ${TPS} SRCS CavePacker.cpp ../../Main.cpp WINDOWED APPNAME "CavePacker" VERSION 1.4 VERSION_CODE 10)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${PROJECT_NAME}-server ${PROJECT_NAME}-client ${PROJECT_NAME}-shared game)
set(LUAFILES config.lua fonts.lua sprites.lua textures/complete.lua)
check_lua_files(${PROJECT_NAME} "${LUAFILES}")
