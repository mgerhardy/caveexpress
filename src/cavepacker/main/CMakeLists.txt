project(cavepacker)
add_executable(${PROJECT_NAME} CavePacker.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-server ${PROJECT_NAME}-client ${PROJECT_NAME}-shared main)
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
