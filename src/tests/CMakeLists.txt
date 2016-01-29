project(tests)
textures(${PROJECT_NAME})
set(${PROJECT_NAME}_SRC
	ByteStreamTest.cpp
	CooldownMgrTest.cpp
	EnumTest.cpp
	FileTest.cpp
	GeneralTest.cpp
	LUATest.cpp
	MemoryAllocatorTest.cpp
	NetworkTest.cpp
	NoNetworkTest.cpp
	ParticleTest.cpp
	SQLiteTest.cpp
	SystemTest.cpp
	TestAll.cpp
	TestGame.cpp
	TestShared.cpp
	TimeManagerTest.cpp
	UITest.cpp)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS ${${PROJECT_NAME}_SRC} APPNAME Tests VERSION 1.0 VERSION_CODE 1)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS common cooldown client game network ui gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
