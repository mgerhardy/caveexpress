set(NAME cavepacker)
set(${NAME}-client_SRCS
	ui/nodes/UICavePackerNodePoint.cpp
	ui/nodes/UINodeMap.cpp
	ui/windows/UICavePackerMapOptionsWindow.cpp
	ui/windows/intro/IntroGame.cpp
	ui/windows/UIMainWindow.cpp
	ui/windows/UIMapWindow.cpp
	CavePackerClientMap.cpp
)
add_library(${NAME}-client ${${NAME}-client_SRCS})
set_target_properties(${NAME}-client PROPERTIES FOLDER ${NAME}-client)
target_link_libraries(${NAME}-client client ${NAME}-shared)
