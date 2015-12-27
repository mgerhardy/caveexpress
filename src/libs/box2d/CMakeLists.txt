set(SRCS
	Common/b2StackAllocator.cpp
	Common/b2BlockAllocator.cpp
	Common/b2Draw.cpp
	Common/b2Timer.cpp
	Common/b2Settings.cpp
	Common/b2Math.cpp
	Collision/b2TimeOfImpact.cpp
	Collision/b2CollidePolygon.cpp
	Collision/b2CollideEdge.cpp
	Collision/b2BroadPhase.cpp
	Collision/b2CollideCircle.cpp
	Collision/b2Collision.cpp
	Collision/b2Distance.cpp
	Collision/Shapes/b2PolygonShape.cpp
	Collision/Shapes/b2ChainShape.cpp
	Collision/Shapes/b2CircleShape.cpp
	Collision/Shapes/b2EdgeShape.cpp
	Collision/b2DynamicTree.cpp
	Dynamics/b2Body.cpp
	Dynamics/b2World.cpp
	Dynamics/b2ContactManager.cpp
	Dynamics/b2Fixture.cpp
	Dynamics/Contacts/b2Contact.cpp
	Dynamics/Contacts/b2ChainAndCircleContact.cpp
	Dynamics/Contacts/b2PolygonAndCircleContact.cpp
	Dynamics/Contacts/b2CircleContact.cpp
	Dynamics/Contacts/b2EdgeAndCircleContact.cpp
	Dynamics/Contacts/b2ContactSolver.cpp
	Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
	Dynamics/Contacts/b2PolygonContact.cpp
	Dynamics/Contacts/b2ChainAndPolygonContact.cpp
	Dynamics/b2WorldCallbacks.cpp
	Dynamics/Joints/b2PrismaticJoint.cpp
	Dynamics/Joints/b2WeldJoint.cpp
	Dynamics/Joints/b2FrictionJoint.cpp
	Dynamics/Joints/b2DistanceJoint.cpp
	Dynamics/Joints/b2GearJoint.cpp
	Dynamics/Joints/b2MotorJoint.cpp
	Dynamics/Joints/b2RopeJoint.cpp
	Dynamics/Joints/b2Joint.cpp
	Dynamics/Joints/b2RevoluteJoint.cpp
	Dynamics/Joints/b2WheelJoint.cpp
	Dynamics/Joints/b2MouseJoint.cpp
	Dynamics/Joints/b2PulleyJoint.cpp
	Dynamics/b2Island.cpp
	Rope/b2Rope.cpp
	)
cp_add_library(LIB box2d SRCS ${SRCS} CFLAGS "-Wno-shadow -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-array-bounds")
# system wide lib must be checked for compatibility if not found via pkg-config
if (BOX2D_FOUND AND ${BOX2D_VERSION} STREQUAL "")
	cp_check_source_compiles("
		#include <Box2D.h>
		int main (int argc, char *argv[]) {
			b2Fixture* fixture;
			return 0;
		}" VAR BOX2D_SUPPORTED CXX INCLUDE_DIRS ${BOX2D_INCLUDE_DIRS} FLAGS ${BOX2D_COMPILER_FLAGS})
	if (NOT BOX2D_SUPPORTED)
		message(FATAL_ERROR "System wide box 2d installation is not supported - we need at least 2.3.0. Please use the bundled version (cmake -DUSE_BUILTIN_BOX2D=TRUE) or upgrade box2d")
	endif()
endif()
