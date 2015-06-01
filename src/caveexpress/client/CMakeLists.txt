set(caveexpress-client_SRCS
	entities/ClientCaveTile.cpp
	entities/ClientWindowTile.cpp
	entities/ClientParticle.cpp
	CaveExpressClientMap.cpp
	ui/nodes/UINodeMapEditorSelectedItem.cpp
	ui/nodes/UINodeMapEditor.cpp
	ui/nodes/UINodeEntitySelector.cpp
	ui/nodes/UINodeSpriteSelector.cpp
	ui/nodes/UINodeBackgroundScene.cpp
	ui/nodes/UINodeMap.cpp
	ui/windows/UIGameOverWindow.cpp
	ui/windows/intro/IntroFlying.cpp
	ui/windows/intro/IntroFindYourWay.cpp
	ui/windows/intro/IntroGeyser.cpp
	ui/windows/intro/IntroTime.cpp
	ui/windows/intro/IntroTree.cpp
	ui/windows/intro/IntroPackage.cpp
	ui/windows/intro/IntroAttack.cpp
	ui/windows/UIGameFinishedWindow.cpp
	ui/windows/UIMapEditorOptionsWindow.cpp
	ui/windows/UIMapFailedWindow.cpp
	ui/windows/UICaveExpressSettingsWindow.cpp
	ui/windows/UIMainWindow.cpp
	ui/windows/UIGameHelpWindow.cpp
	ui/windows/UIMapEditorHelpWindow.cpp
	ui/windows/UIMapWindow.cpp
	ui/windows/UIMapEditorWindow.cpp
	ui/windows/UIHelpWindow.cpp
)
add_library(caveexpress-client ${caveexpress-client_SRCS})
set_target_properties(caveexpress-client PROPERTIES FOLDER caveexpress-client)
target_link_libraries(caveexpress-client caveexpress-shared libs client server common game network ports)