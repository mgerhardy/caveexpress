#pragma once

#include "caveexpress/server/entities/IEntity.h"

namespace caveexpress {

// forward decl
class CaveMapTile;

/**
 * @brief Platform body that is automatically created where NPCs are moving on.
 * @sa CaveMapTile
 */
class Platform: public IEntity {
private:
	CaveMapTile* _caveTile;

public:
	explicit Platform (Map& map);
	virtual ~Platform ();

	CaveMapTile *getCave () const;
	void setCave (CaveMapTile* cave);

	// IEntity
	SpriteDefPtr getSpriteDef () const override;
	bool shouldCollide (const IEntity* entity) const override;
	void onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
};

typedef std::shared_ptr<Platform> PlatformPtr;

inline CaveMapTile *Platform::getCave () const
{
	return _caveTile;
}

inline void Platform::setCave (CaveMapTile* cave)
{
	_caveTile = cave;
}

}
