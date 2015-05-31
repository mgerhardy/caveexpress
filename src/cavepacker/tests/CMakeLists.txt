project(tests-cavepacker)
SET(${PROJECT_NAME}_SRC
	MapManagerTest.cpp
	ProtocolTest.cpp
	SokobanMapContextTest.cpp
	SokobanMapTest.cpp
	CampaignTest.cpp
)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports)

#	TestAll.cpp
#	TestShared.cpp
#	NetworkTest.cpp
#	NoNetworkTest.cpp
