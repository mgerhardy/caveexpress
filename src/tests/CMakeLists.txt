project(tests)
file(GLOB ${PROJECT_NAME}_SRC
	FileTest.cpp
	ByteStreamTest.cpp
	ParticleTest.cpp
	TimeManagerTest.cpp
	TestShared.cpp
	SystemTest.cpp
	SQLiteTest.cpp
	GeneralTest.cpp
	TestAll.cpp
	LUATest.cpp
	UITest.cpp
	MemoryAllocatorTest.cpp
	EnumTest.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports ui)

project(tests-caveexpress)
SET(${PROJECT_NAME}_SRC
	TestAll.cpp
	TestShared.cpp
	NetworkTest.cpp
	NoNetworkTest.cpp
	caveexpress/ProtocolTest.cpp
)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports)
#	caveexpress/EnumSpriteTypesTest.cpp
#	caveexpress/LUASpriteTest.cpp
#	caveexpress/RandomMapContextTest.cpp
#	caveexpress/MapTest.cpp
#	caveexpress/CampaignTest.cpp

project(tests-cavepacker)
SET(${PROJECT_NAME}_SRC
	TestAll.cpp
	TestShared.cpp
	NetworkTest.cpp
	NoNetworkTest.cpp
	cavepacker/MapManagerTest.cpp
	cavepacker/ProtocolTest.cpp
)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports)
#	cavepacker/SokobanMapContextTest.cpp
#	cavepacker/SokobanMapTest.cpp
#	cavepacker/CampaignTest.cpp
