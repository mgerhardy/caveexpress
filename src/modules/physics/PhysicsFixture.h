#pragma once

#include "physics/PhysicsTypes.h"
#include "physics/PhysicsBody.h"

class PhysicsShape;

class PhysicsFixture {
public:
	PhysicsFixture () = default;

	bool isValid () const { return _valid; }
	explicit operator bool () const { return _valid; }

	bool operator== (const PhysicsFixture& other) const
	{
		return _valid == other._valid && _key == other._key && _aux == other._aux;
	}

	bool operator!= (const PhysicsFixture& other) const
	{
		return !(*this == other);
	}

	bool operator< (const PhysicsFixture& other) const
	{
		if (_key != other._key)
			return _key < other._key;
		return _aux < other._aux;
	}

	PhysicsBody getBody () const;
	PhysicsShapeType getShapeType () const;
	float getDensity () const;
	PhysicsFilter getFilterData () const;
	PhysicsUserData getUserData () const;
	void setUserData (PhysicsUserData data) const;
	void refilter () const;

	PhysicsFixture getNext () const;

	/// Polygon vertex count (0 if not a polygon).
	int getPolygonVertexCount () const;
	PhysicsVec2 getPolygonVertex (int index) const;

	/// Circle radius / position in local body space (0 if not a circle).
	float getCircleRadius () const;
	PhysicsVec2 getCircleLocalCenter () const;

	int getChildCount () const;
	void computeAABB (PhysicsAABB& aabb, const PhysicsTransform& xf, int childIndex) const;

	uint64_t key () const { return _key; }
	uint64_t aux () const { return _aux; }

	static PhysicsFixture fromStorage (uint64_t key, uint64_t aux = 0)
	{
		PhysicsFixture f;
		f._key = key;
		f._aux = aux;
		f._valid = (key != 0 || aux != 0);
		return f;
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
	friend class PhysicsContact;

	uint64_t _key = 0;
	uint64_t _aux = 0;
	bool _valid = false;
};

struct PhysicsFixtureDef {
	PhysicsShapeType shapeType = PhysicsShapeType::Polygon;
	/// Polygon / edge vertices (edge uses first two).
	PhysicsVec2 vertices[PhysicsMaxPolygonVertices];
	int vertexCount = 0;
	/// Circle
	PhysicsVec2 circleCenter;
	float radius = 0.0f;
	/// Box helper: if set, overrides vertices with a box at origin.
	bool useBox = false;
	float boxHalfWidth = 0.0f;
	float boxHalfHeight = 0.0f;

	float density = 0.0f;
	float friction = 0.2f;
	float restitution = 0.0f;
	bool isSensor = false;
	PhysicsFilter filter;
	PhysicsUserData userData = 0;
	/// Enable mid-step pre-solve events (needed for contact disable on 3.x).
	bool enablePreSolveEvents = true;
	bool enableContactEvents = true;
};
