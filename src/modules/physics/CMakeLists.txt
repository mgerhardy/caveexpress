set(physics_SRCS DebugRenderer.cpp)

if (NOT DEFINED CP_BOX2D_MAJOR)
	set(CP_BOX2D_MAJOR 2)
endif()

if (CP_BOX2D_MAJOR GREATER_EQUAL 3)
	list(APPEND physics_SRCS box2d3/PhysicsBackend.cpp)
else()
	list(APPEND physics_SRCS box2d24/PhysicsBackend.cpp)
endif()

add_library(physics ${physics_SRCS})
set_target_properties(physics PROPERTIES FOLDER physics)
target_compile_definitions(physics PUBLIC CP_BOX2D_MAJOR=${CP_BOX2D_MAJOR})
cp_target_link_libraries(TARGET physics LIBS common box2d)
message(STATUS "physics: building Box2D ${CP_BOX2D_MAJOR}.x backend")
