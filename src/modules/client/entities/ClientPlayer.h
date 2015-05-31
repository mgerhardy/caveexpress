#pragma once

#include "client/entities/ClientEntity.h"

class ClientPlayer: public ClientEntity {
private:
	const EntityType *_hasCollected;

protected:
	ClientPlayer (const EntityType& type, uint16_t id, const Animation& animation, float x, float y, float sizeX,
			float sizeY, const SoundMapping& soundMapping, EntityAlignment align);
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const;
	};
	static Factory FACTORY;
	virtual ~ClientPlayer ();

	inline bool hasCollected () const
	{
		return !_hasCollected->isNone();
	}

	inline void setCollected (const EntityType &type)
	{
		_hasCollected = &type;
	}
};
