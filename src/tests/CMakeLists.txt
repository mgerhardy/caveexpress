project(tests)
textures(${PROJECT_NAME})
set(${PROJECT_NAME}_SRC
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
cp_add_executable(TARGET ${PROJECT_NAME} SRCS ${${PROJECT_NAME}_SRC})
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS common client game network ui gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
