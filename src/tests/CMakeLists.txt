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
	MemoryAllocatorTest.cpp
	EnumTest.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports)

#UITest.cpp

if(FALSE)
project(tests-caveexpress)
SET(${PROJECT_NAME}_SRC
	TestAll.cpp
	NetworkTest.cpp
	NoNetworkTest.cpp
	caveexpress/ProtocolTest.cpp
	caveexpress/MapTest.cpp
	caveexpress/CampaignTest.cpp
	caveexpress/EnumSpriteTypesTest.cpp
	caveexpress/LUASpriteTest.cpp
	caveexpress/RandomMapContextTest.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} modules)

project(tests-cavepacker)
SET(${PROJECT_NAME}_SRC
	TestAll.cpp
	NetworkTest.cpp
	NoNetworkTest.cpp
	cavepacker/MapManagerTest.cpp
	cavepacker/ProtocolTest.cpp
	cavepacker/CampaignTest.cpp
	cavepacker/SokobanMapContextTest.cpp
	cavepacker/SokobanMapTest.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} modules)
endif()

