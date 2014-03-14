#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include <string>

// forward decl
class Map;

class EntityEmitter: public IEntity {
private:
	// the position of the emitter
	float _x;
	float _y;
	// the overall amount of emits before this emitter will get removed
	int _amount;
	// the delay between several emits
	int _delay;
	// next spawn timer
	int _spawnTimer;
	// the type of entity to emit
	const EntityType& _type;
	// the amount of already emitted entities
	int _count;
	// key=value,key=value
	std::string _settings;

	b2Vec2 getRealPos (const EntityType &entityType) const;
public:
	EntityEmitter (Map& map, float x, float y, int amount, int delay, const EntityType& type, const std::string& settings);
	virtual ~EntityEmitter ();

	static const EntityType** getSupportedEntityTypes();

	// IEntity
	bool shouldCollide (const IEntity* entity) const override;
	void update (uint32_t deltaTime) override;

	// IEntity
	bool isRemove () const override;
};
