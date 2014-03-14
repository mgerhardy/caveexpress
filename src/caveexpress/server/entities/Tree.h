#pragma once

#include "caveexpress/server/entities/IEntity.h"

class Map;

class Tree: public IEntity {
private:
	gridCoord _x;
	gridCoord _y;
	bool _dropFruit;
	int _droppedFruits;
	IEntity *_dropFruitCausedBy;
	TimerID _idleTimer;

public:
	Tree (Map& map, gridCoord x, gridCoord y);
	virtual ~Tree ();

	/**
	 * @param[in] entity The entity that has dazed this entity
	 */
	void setDazed (IEntity* entity);

	void setIdle ();

	void createBody ();

	// IEntity
	bool shouldCollide (const IEntity *entity) const override;
	void onContact (b2Contact* contact, IEntity* entity) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
	void update (uint32_t deltaTime) override;
};
