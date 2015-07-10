set(NAME miniracer)
set(${NAME}-client_SRCS
	ui/nodes/UINodeMap.cpp
	ui/windows/UIMiniRacerMapOptionsWindow.cpp
	ui/windows/UIMainWindow.cpp
	ui/windows/UIMapWindow.cpp
	MiniRacerClientMap.cpp
)
add_library(${NAME}-client ${${NAME}-client_SRCS})
set_target_properties(${NAME}-client PROPERTIES FOLDER ${NAME}-client)
cp_target_link_libraries(TARGET ${NAME}-client LIBS client ${NAME}-shared)
