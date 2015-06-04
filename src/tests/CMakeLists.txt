project(tests)
textures(${PROJECT_NAME})
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
	TestGame.cpp
	UITest.cpp
	MemoryAllocatorTest.cpp
	EnumTest.cpp)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} common client game network ports ui)
