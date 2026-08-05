set(NAME cavepacker)
project(tests_${NAME})
SET(${PROJECT_NAME}_SRC
	BoardStateTest.cpp
	MapManagerTest.cpp
	ProtocolTest.cpp
	SokobanMapContextTest.cpp
	SokobanMapTest.cpp
	CampaignTest.cpp
	PathfindingTest.cpp
	SolutionUtilTest.cpp
	../main/CavePacker.cpp
	../../tests/TestAll.cpp
	../../tests/TestShared.cpp
	../../tests/NetworkTest.cpp
	../../tests/NoNetworkTest.cpp
)
cp_add_executable(TARGET ${PROJECT_NAME} SRCS ${${PROJECT_NAME}_SRC} APPNAME ${PROJECT_NAME} VERSION 1.0 VERSION_CODE 1)
cp_target_link_libraries(TARGET ${PROJECT_NAME} LIBS ${NAME}-client ${NAME}-server ${NAME}-shared gtest)
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE -bigobj)
endif()

# Non-SokobanMapTest suites (fast). Sokoban map replay suites are split below so
# ctest -j and per-batch timeouts can work.
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} --gtest_filter=-SokobanMapTest.* WORKING_DIRECTORY ${ROOT_DIR})
set_tests_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 120)

# Sokoban map families — one ctest entry each for parallel execution
set(_sokoban_batches
	"xsokoban|SokobanMapTest.testXSokoban*"
	"microban|SokobanMapTest.testmicroban*"
	"sasquatch|SokobanMapTest.testsasquatch*"
	"gri|SokobanMapTest.testgri*:SokobanMapTest.testgrigrspecial*"
	"misc|SokobanMapTest.testTutorials:SokobanMapTest.testmgerhardy*:SokobanMapTest.testSolution*:SokobanMapTest.testUndo*"
)
foreach (_batch ${_sokoban_batches})
	string(REPLACE "|" ";" _parts "${_batch}")
	list(GET _parts 0 _suffix)
	list(GET _parts 1 _filter)
	set(_test_name ${PROJECT_NAME}_sokoban_${_suffix})
	add_test(NAME ${_test_name} COMMAND ${PROJECT_NAME} --gtest_filter=${_filter} WORKING_DIRECTORY ${ROOT_DIR})
	# Long solutions with many steps; deadlock work is skipped during autosolve now,
	# but map load + network setup per case still needs headroom on CI.
	set_tests_properties(${_test_name} PROPERTIES TIMEOUT 300)
endforeach()
