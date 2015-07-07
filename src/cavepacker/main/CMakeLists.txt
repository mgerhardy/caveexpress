project(cavepacker)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS CavePacker.cpp ../../Main.cpp WINDOWED)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${PROJECT_NAME}-server ${PROJECT_NAME}-client ${PROJECT_NAME}-shared server game common service)
set(TPS
	cavepacker-entity
	cavepacker-environment
	cavepacker-ui-fonts
	cavepacker-ui-scene
	cavepacker-ui
)
texturepacker(${PROJECT_NAME} "${TPS}")
textures(${PROJECT_NAME})
create_dir_header(${PROJECT_NAME})
prepare_android(${PROJECT_NAME} "CavePacker" 1.4 9)
