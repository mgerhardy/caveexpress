set(NAME caveexpress)
project(tests-${NAME})
SET(${PROJECT_NAME}_SRC
	ProtocolTest.cpp
	EnumSpriteTypesTest.cpp
	LUASpriteTest.cpp
	RandomMapContextTest.cpp
	MapTest.cpp
	MapManagerTest.cpp
	CampaignTest.cpp
	../main/CaveExpress.cpp
	../../tests/TestAll.cpp
	../../tests/TestShared.cpp
	../../tests/NetworkTest.cpp
	../../tests/NoNetworkTest.cpp
)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS ${${PROJECT_NAME}_SRC} APPNAME ${PROJECT_NAME} VERSION 1.0 VERSION_CODE 1)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${NAME}-client ${NAME}-server ${NAME}-shared gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
