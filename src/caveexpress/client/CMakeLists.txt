set(NAME caveexpress)
set(${NAME}-client_SRCS
	entities/ClientCaveTile.cpp
	entities/ClientNPC.cpp
	entities/ClientWindowTile.cpp
	entities/ClientParticle.cpp
	CaveExpressClientMap.cpp
	ui/nodes/UINodeMapEditor.cpp
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
)
add_library(${NAME}-client ${${NAME}-client_SRCS})
set_target_properties(${NAME}-client PROPERTIES FOLDER ${NAME}-client)
cp_target_link_libraries(TARGET ${NAME}-client LIBS client ${NAME}-shared)
