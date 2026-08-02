#pragma once

#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"

class PhysicsJoint {
public:
	PhysicsJoint () = default;

	bool isValid () const { return _valid; }
	explicit operator bool () const { return _valid; }

	bool operator== (const PhysicsJoint& other) const
	{
		return _valid == other._valid && _key == other._key && _aux == other._aux;
	}

	bool operator!= (const PhysicsJoint& other) const
	{
		return !(*this == other);
	}

	PhysicsBody getBodyA () const;
	PhysicsBody getBodyB () const;
	PhysicsUserData getUserData () const;
	void setUserData (PhysicsUserData data) const;

	/// Distance joint
	float getLength () const;
	void setLength (float length) const;

	/// Revolute joint
	void setMotorSpeed (float speed) const;

	uint64_t key () const { return _key; }
	uint64_t aux () const { return _aux; }

	static PhysicsJoint fromStorage (uint64_t key, uint64_t aux = 0)
	{
		PhysicsJoint j;
		j._key = key;
		j._aux = aux;
		j._valid = (key != 0 || aux != 0);
		return j;
	}

	void clear ()
	{
		_key = 0;
		_aux = 0;
		_valid = false;
	}

private:
	friend class PhysicsWorld;
	uint64_t _key = 0;
	uint64_t _aux = 0;
	bool _valid = false;
};

struct PhysicsDistanceJointDef {
	PhysicsBody bodyA;
	PhysicsBody bodyB;
	PhysicsVec2 localAnchorA;
	PhysicsVec2 localAnchorB;
	float minLength = 0.0f;
	float maxLength = 0.0f;
	bool collideConnected = false;
	PhysicsUserData userData = 0;
	/// If true, initialize local anchors from world points.
	bool useWorldAnchors = false;
	PhysicsVec2 worldAnchorA;
	PhysicsVec2 worldAnchorB;
};

struct PhysicsRevoluteJointDef {
	PhysicsBody bodyA;
	PhysicsBody bodyB;
	PhysicsVec2 localAnchorA;
	PhysicsVec2 localAnchorB;
	float lowerAngle = 0.0f;
	float upperAngle = 0.0f;
	bool enableLimit = false;
	bool enableMotor = false;
	float motorSpeed = 0.0f;
	float maxMotorTorque = 0.0f;
	bool collideConnected = false;
	PhysicsUserData userData = 0;
	/// If true, initialize local anchors from a shared world pivot.
	bool useWorldPivot = false;
	PhysicsVec2 worldPivot;
};
