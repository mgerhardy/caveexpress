set(cavepacker-client_SRCS
	ui/nodes/UICavePackerNodePoint.cpp
	ui/nodes/UINodeMap.cpp
	ui/windows/UICavePackerMapOptionsWindow.cpp
	ui/windows/intro/IntroGame.cpp
	ui/windows/UIMainWindow.cpp
	ui/windows/UIMapWindow.cpp
	CavePackerClientMap.cpp
)
add_library(cavepacker-client ${cavepacker-client_SRCS})
set_target_properties(cavepacker-client PROPERTIES FOLDER cavepacker-client)
target_link_libraries(cavepacker-client cavepacker-shared libs client server common game network ports)