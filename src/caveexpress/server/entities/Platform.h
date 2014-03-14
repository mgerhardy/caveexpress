#pragma once

#include "caveexpress/server/entities/IEntity.h"

// forward decl
class CaveMapTile;

class Platform: public IEntity {
private:
	CaveMapTile* _caveTile;

public:
	Platform (Map& map);
	virtual ~Platform ();

	CaveMapTile *getCave () const;
	void setCave (CaveMapTile* cave);

	// IEntity
	SpriteDefPtr getSpriteDef () const override;
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
};

typedef SharedPtr<Platform> PlatformPtr;

inline CaveMapTile *Platform::getCave () const
{
	return _caveTile;
}

inline void Platform::setCave (CaveMapTile* cave)
{
	_caveTile = cave;
}
