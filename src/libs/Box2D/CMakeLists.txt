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
find_package(Box2D)
if (NOT BOX2D_FOUND)
	set(BOX2D_LIBRARIES Box2D)
	add_library(Box2D ${SRCS})
	set(BOX2D_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/libs/Box2D")
	set_target_properties(Box2D PROPERTIES FOLDER Box2D)
	set_target_properties(Box2D PROPERTIES COMPILE_FLAGS "-Wno-shadow")
endif()
include_directories(${BOX2D_INCLUDE_DIRS})
