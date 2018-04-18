set(SRCS
	Box2D/Common/b2StackAllocator.cpp
	Box2D/Common/b2BlockAllocator.cpp
	Box2D/Common/b2Draw.cpp
	Box2D/Common/b2Timer.cpp
	Box2D/Common/b2Settings.cpp
	Box2D/Common/b2Math.cpp
	Box2D/Collision/b2TimeOfImpact.cpp
	Box2D/Collision/b2CollidePolygon.cpp
	Box2D/Collision/b2CollideEdge.cpp
	Box2D/Collision/b2BroadPhase.cpp
	Box2D/Collision/b2CollideCircle.cpp
	Box2D/Collision/b2Collision.cpp
	Box2D/Collision/b2Distance.cpp
	Box2D/Collision/Shapes/b2PolygonShape.cpp
	Box2D/Collision/Shapes/b2ChainShape.cpp
	Box2D/Collision/Shapes/b2CircleShape.cpp
	Box2D/Collision/Shapes/b2EdgeShape.cpp
	Box2D/Collision/b2DynamicTree.cpp
	Box2D/Dynamics/b2Body.cpp
	Box2D/Dynamics/b2World.cpp
	Box2D/Dynamics/b2ContactManager.cpp
	Box2D/Dynamics/b2Fixture.cpp
	Box2D/Dynamics/Contacts/b2Contact.cpp
	Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
	Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
	Box2D/Dynamics/Contacts/b2CircleContact.cpp
	Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
	Box2D/Dynamics/Contacts/b2ContactSolver.cpp
	Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
	Box2D/Dynamics/Contacts/b2PolygonContact.cpp
	Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
	Box2D/Dynamics/b2WorldCallbacks.cpp
	Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
	Box2D/Dynamics/Joints/b2WeldJoint.cpp
	Box2D/Dynamics/Joints/b2FrictionJoint.cpp
	Box2D/Dynamics/Joints/b2DistanceJoint.cpp
	Box2D/Dynamics/Joints/b2GearJoint.cpp
	Box2D/Dynamics/Joints/b2MotorJoint.cpp
	Box2D/Dynamics/Joints/b2RopeJoint.cpp
	Box2D/Dynamics/Joints/b2Joint.cpp
	Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
	Box2D/Dynamics/Joints/b2WheelJoint.cpp
	Box2D/Dynamics/Joints/b2MouseJoint.cpp
	Box2D/Dynamics/Joints/b2PulleyJoint.cpp
	Box2D/Dynamics/b2Island.cpp
	Box2D/Rope/b2Rope.cpp
	)
cp_add_library(LIB box2d SRCS ${SRCS} CFLAGS "-Wno-shadow -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Wno-array-bounds")
# system wide lib must be checked for compatibility if not found via pkg-config
if (BOX2D_FOUND AND "${BOX2D_VERSION}" STREQUAL "")
	cp_check_source_compiles("
		#include <Box2D/Box2D.h>
		int main (int argc, char *argv[]) {
			b2Fixture* fixture;
			return 0;
		}" VAR BOX2D_SUPPORTED CXX INCLUDE_DIRS ${BOX2D_INCLUDE_DIRS} FLAGS ${BOX2D_COMPILER_FLAGS})
	if (NOT BOX2D_SUPPORTED)
		message(FATAL_ERROR "System wide box 2d installation is not supported - we need at least 2.3.0. Please use the bundled version (cmake -DUSE_BUILTIN_BOX2D=TRUE) or upgrade box2d")
	endif()
elseif (NOT BOX2D_FOUND)
	# If not found system wide, we have to include the bundled header directory.
	target_include_directories(box2d PUBLIC Box2D)
endif()
