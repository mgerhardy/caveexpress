#pragma once

#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"
#include "physics/PhysicsFixture.h"

class PhysicsContact {
public:
	PhysicsContact () = default;

	bool isValid () const { return _valid; }
	explicit operator bool () const { return _valid; }

	bool operator== (const PhysicsContact& other) const
	{
		return _valid == other._valid && _key == other._key && _aux == other._aux;
	}

	bool operator!= (const PhysicsContact& other) const
	{
		return !(*this == other);
	}

	PhysicsFixture getFixtureA () const;
	PhysicsFixture getFixtureB () const;
	PhysicsManifold getManifold () const;
	void getWorldManifold (PhysicsWorldManifold& manifold) const;
	bool isTouching () const;
	void setEnabled (bool enabled) const;

	uint64_t key () const { return _key; }
	uint64_t aux () const { return _aux; }

	static PhysicsContact fromStorage (uint64_t key, uint64_t aux = 0)
	{
		PhysicsContact c;
		c._key = key;
		c._aux = aux;
		c._valid = (key != 0 || aux != 0);
		return c;
	}

	void clear ()
	{
		_key = 0;
		_aux = 0;
		_valid = false;
	}

private:
	friend class PhysicsWorld;
	friend class PhysicsBody;
	friend class PhysicsContactEdge;
	uint64_t _key = 0;
	uint64_t _aux = 0;
	bool _valid = false;
};

class PhysicsContactEdge {
public:
	PhysicsContactEdge () = default;

	bool isValid () const { return _valid; }
	explicit operator bool () const { return _valid; }

	PhysicsContact contact;
	PhysicsContactEdge next () const;

	static PhysicsContactEdge fromStorage (uint64_t key, uint64_t aux = 0)
	{
		PhysicsContactEdge e;
		e._key = key;
		e._aux = aux;
		e._valid = (key != 0 || aux != 0);
		return e;
	}

private:
	friend class PhysicsBody;
	uint64_t _key = 0;
	uint64_t _aux = 0;
	bool _valid = false;
};

void physGetPointStates (PhysicsPointState state1[PhysicsMaxManifoldPoints],
		PhysicsPointState state2[PhysicsMaxManifoldPoints],
		const PhysicsManifold& manifold1, const PhysicsManifold& manifold2);
