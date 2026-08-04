#pragma once

#include "client/entities/ClientMapTile.h"

namespace caveexpress {

class ClientPressurePlate: public ClientMapTile {
private:
	ClientPressurePlate (uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align);

public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const override;
	};
	static Factory FACTORY;

	virtual ~ClientPressurePlate ();

	void setAnimationType (const Animation& animation) override;
};

}
