project(tests-caveexpress)
SET(${PROJECT_NAME}_SRC
	ProtocolTest.cpp
	EnumSpriteTypesTest.cpp
	LUASpriteTest.cpp
	RandomMapContextTest.cpp
	MapTest.cpp
	CampaignTest.cpp
)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports server)

#	TestAll.cpp
#	TestShared.cpp
#	NetworkTest.cpp
#	NoNetworkTest.cpp
