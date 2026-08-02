#pragma once

#include "physics/PhysicsTypes.h"

class PhysicsFixture;
class PhysicsContact;
class PhysicsWorld;

class PhysicsBody {
public:
	PhysicsBody () = default;

	bool isValid () const { return _valid; }
	explicit operator bool () const { return _valid; }

	bool operator== (const PhysicsBody& other) const
	{
		return _valid == other._valid && _key == other._key && _aux == other._aux;
	}

	bool operator!= (const PhysicsBody& other) const
	{
		return !(*this == other);
	}

	bool operator< (const PhysicsBody& other) const
	{
		if (_key != other._key)
			return _key < other._key;
		return _aux < other._aux;
	}

	PhysicsVec2 getPosition () const;
	float getAngle () const;
	PhysicsVec2 getWorldCenter () const;
	PhysicsVec2 getWorldPoint (const PhysicsVec2& localPoint) const;
	PhysicsVec2 getLinearVelocity () const;
	float getAngularVelocity () const;
	float getInertia () const;
	float getMass () const;
	float getGravityScale () const;
	PhysicsVec2 getLinearVelocityFromWorldPoint (const PhysicsVec2& worldPoint) const;
	PhysicsUserData getUserData () const;
	PhysicsWorld* getWorld () const;

	void setTransform (const PhysicsVec2& position, float angle) const;
	void setLinearVelocity (const PhysicsVec2& v) const;
	void setAngularVelocity (float omega) const;
	void setGravityScale (float scale) const;
	void setLinearDamping (float damping) const;
	void setAngularDamping (float damping) const;
	void setFixedRotation (bool flag) const;
	void setEnabled (bool flag) const;
	void setUserData (PhysicsUserData data) const;

	void applyForce (const PhysicsVec2& force, const PhysicsVec2& point, bool wake = true) const;
	void applyForceToCenter (const PhysicsVec2& force, bool wake = true) const;
	void applyLinearImpulse (const PhysicsVec2& impulse, const PhysicsVec2& point, bool wake = true) const;
	void applyTorque (float torque, bool wake = true) const;

	PhysicsFixture createFixture (const class PhysicsFixtureDef& def) const;
	PhysicsFixture getFixtureList () const;
	/// First contact edge for this body (iterate with PhysicsContactEdge::next).
	class PhysicsContactEdge getContactList () const;

	/// Opaque storage accessors used by backends.
	uint64_t key () const { return _key; }
	uint64_t aux () const { return _aux; }

	static PhysicsBody fromStorage (uint64_t key, uint64_t aux = 0)
	{
		PhysicsBody b;
		b._key = key;
		b._aux = aux;
		b._valid = (key != 0 || aux != 0);
		return b;
	}

	void clear ()
	{
		_key = 0;
		_aux = 0;
		_valid = false;
	}

private:
	friend class PhysicsWorld;
	friend class PhysicsFixture;
	friend class PhysicsContact;
	friend class PhysicsJoint;
	friend class PhysicsContactEdge;

	uint64_t _key = 0;
	uint64_t _aux = 0;
	bool _valid = false;
};

struct PhysicsBodyDef {
	PhysicsBodyType type = PhysicsBodyType::Static;
	PhysicsVec2 position;
	float angle = 0.0f;
	PhysicsVec2 linearVelocity;
	float angularVelocity = 0.0f;
	float linearDamping = 0.0f;
	float angularDamping = 0.0f;
	bool fixedRotation = false;
	bool bullet = false;
	float gravityScale = 1.0f;
	PhysicsUserData userData = 0;
};
