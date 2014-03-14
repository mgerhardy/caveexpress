#pragma once

#include "caveexpress/server/entities/IEntity.h"

class Map;

namespace BorderType {
typedef enum {
	TOP, LEFT, RIGHT, BOTTOM, PLAYER_BOTTOM
} Type;
}

class Border: public IEntity {
private:
	BorderType::Type _borderType;
	bool _crashOnTouch;

public:
	Border (BorderType::Type borderType, Map& map, bool crashOnTouch = false);
	virtual ~Border ();

	bool isTop () const;
	bool isLeft () const;
	bool isRight () const;
	bool isBottom () const;

	// IEntity
	bool shouldCollide (const IEntity *entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
};

inline bool Border::isTop () const
{
	return _borderType == BorderType::TOP;
}

inline bool Border::isLeft () const
{
	return _borderType == BorderType::LEFT;
}

inline bool Border::isRight () const
{
	return _borderType == BorderType::RIGHT;
}

inline bool Border::isBottom () const
{
	return _borderType == BorderType::BOTTOM;
}
