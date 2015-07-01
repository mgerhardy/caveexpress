set(NAME caveexpress)
project(tests-${NAME})
SET(${PROJECT_NAME}_SRC
	ProtocolTest.cpp
	EnumSpriteTypesTest.cpp
	LUASpriteTest.cpp
	RandomMapContextTest.cpp
	MapTest.cpp
	CampaignTest.cpp
	../main/CaveExpress.cpp
	../../tests/TestAll.cpp
	../../tests/TestShared.cpp
	../../tests/NetworkTest.cpp
	../../tests/NoNetworkTest.cpp
)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${NAME}-client ${NAME}-server ${NAME}-shared gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
