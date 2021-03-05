set(SRCS
	box2d/collision/b2_broad_phase.cpp
	box2d/collision/b2_chain_shape.cpp
	box2d/collision/b2_circle_shape.cpp
	box2d/collision/b2_collide_circle.cpp
	box2d/collision/b2_collide_edge.cpp
	box2d/collision/b2_collide_polygon.cpp
	box2d/collision/b2_collision.cpp
	box2d/collision/b2_distance.cpp
	box2d/collision/b2_dynamic_tree.cpp
	box2d/collision/b2_edge_shape.cpp
	box2d/collision/b2_polygon_shape.cpp
	box2d/collision/b2_time_of_impact.cpp
	box2d/common/b2_block_allocator.cpp
	box2d/common/b2_draw.cpp
	box2d/common/b2_math.cpp
	box2d/common/b2_settings.cpp
	box2d/common/b2_stack_allocator.cpp
	box2d/common/b2_timer.cpp
	box2d/dynamics/b2_body.cpp
	box2d/dynamics/b2_chain_circle_contact.cpp
	box2d/dynamics/b2_chain_circle_contact.h
	box2d/dynamics/b2_chain_polygon_contact.cpp
	box2d/dynamics/b2_chain_polygon_contact.h
	box2d/dynamics/b2_circle_contact.cpp
	box2d/dynamics/b2_circle_contact.h
	box2d/dynamics/b2_contact.cpp
	box2d/dynamics/b2_contact_manager.cpp
	box2d/dynamics/b2_contact_solver.cpp
	box2d/dynamics/b2_contact_solver.h
	box2d/dynamics/b2_distance_joint.cpp
	box2d/dynamics/b2_edge_circle_contact.cpp
	box2d/dynamics/b2_edge_circle_contact.h
	box2d/dynamics/b2_edge_polygon_contact.cpp
	box2d/dynamics/b2_edge_polygon_contact.h
	box2d/dynamics/b2_fixture.cpp
	box2d/dynamics/b2_friction_joint.cpp
	box2d/dynamics/b2_gear_joint.cpp
	box2d/dynamics/b2_island.cpp
	box2d/dynamics/b2_island.h
	box2d/dynamics/b2_joint.cpp
	box2d/dynamics/b2_motor_joint.cpp
	box2d/dynamics/b2_mouse_joint.cpp
	box2d/dynamics/b2_polygon_circle_contact.cpp
	box2d/dynamics/b2_polygon_circle_contact.h
	box2d/dynamics/b2_polygon_contact.cpp
	box2d/dynamics/b2_polygon_contact.h
	box2d/dynamics/b2_prismatic_joint.cpp
	box2d/dynamics/b2_pulley_joint.cpp
	box2d/dynamics/b2_revolute_joint.cpp
	box2d/dynamics/b2_weld_joint.cpp
	box2d/dynamics/b2_wheel_joint.cpp
	box2d/dynamics/b2_world.cpp
	box2d/dynamics/b2_world_callbacks.cpp
	box2d/rope/b2_rope.cpp
	)
include_directories(BEFORE ${ROOT_DIR}/src/modules/physics)
cp_add_library(LIB box2d SRCS ${SRCS} DEFINES B2_USER_SETTINGS CFLAGS "-Wno-shadow -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-array-bounds")
# system wide lib must be checked for compatibility if not found via pkg-config
if (BOX2D_FOUND AND "${BOX2D_VERSION}" STREQUAL "")
	cp_check_source_compiles("
		#include <box2d/box2d.h>
		int main (int argc, char *argv[]) {
			b2Fixture* fixture;
			return 0;
		}" VAR BOX2D_SUPPORTED CXX INCLUDE_DIRS ${BOX2D_INCLUDE_DIRS} FLAGS ${BOX2D_COMPILER_FLAGS})
	if (NOT BOX2D_SUPPORTED)
		message(FATAL_ERROR "System wide box 2d installation is not supported - we need at least 2.4.1. Please use the bundled version (cmake -DUSE_BUILTIN_BOX2D=TRUE) or upgrade box2d")
	endif()
elseif (NOT BOX2D_FOUND)
	# If not found system wide, we have to include the bundled header directory.
	target_include_directories(box2d PUBLIC box2d)
endif()
