set(NAME miniracer)
project(tests_${NAME})
set(${PROJECT_NAME}_SRC
	ProtocolTest.cpp
	LUASpriteTest.cpp
	MiniRacerMapContextTest.cpp
	MapTest.cpp
	MapManagerTest.cpp
	../main/MiniRacer.cpp
	../../tests/TestAll.cpp
	../../tests/TestShared.cpp
	../../tests/NetworkTest.cpp
	../../tests/NoNetworkTest.cpp
)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS ${${PROJECT_NAME}_SRC} APPNAME ${PROJECT_NAME} VERSION 1.0 VERSION_CODE 1)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${NAME}-client ${NAME}-server ${NAME}-shared gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
