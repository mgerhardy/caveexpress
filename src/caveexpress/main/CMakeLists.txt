project(caveexpress)
add_executable(${PROJECT_NAME} CaveExpress.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${PROJECT_NAME}-server ${PROJECT_NAME}-client ${PROJECT_NAME}-shared main)
set(TPS
	caveexpress-entity
	caveexpress-environment-ice
	caveexpress-environment
	caveexpress-npc_blowing
	caveexpress-npc_fish
	caveexpress-npc_flying
	caveexpress-npc_grandpa
	caveexpress-npc_mammut
	caveexpress-npc_man
	caveexpress-npc_walking
	caveexpress-npc_woman
	caveexpress-packagetarget-ice
	caveexpress-packagetarget-rock
	caveexpress-ui-dead
	caveexpress-ui-fonts
	caveexpress-ui-npc-mammut
	caveexpress-ui-player-grandpa
	caveexpress-ui-scene
	caveexpress-ui
)
texturepacker(${PROJECT_NAME} "${TPS}")
textures(${PROJECT_NAME})
set(LUAFILES config.lua entities.lua entitysounds.lua fonts.lua sprites.lua textures/complete.lua)
check_lua_files(${PROJECT_NAME} "${LUAFILES}")
create_dir_header(${PROJECT_NAME})
prepare_android(${PROJECT_NAME} "CaveExpress" 2.4 30)
