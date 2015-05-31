#pragma once

#include "client/entities/ClientMapTile.h"

class ClientCaveTile: public ClientMapTile {
private:
	ClientCaveTile (uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align);

	bool _lightState;
	std::string _sprite;

public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const;
	};
	static Factory FACTORY;

	virtual ~ClientCaveTile ();

	void setLightState (bool lightState);
	bool isLightState () const;

	// ClientEntity
	void render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY) const override;
};

inline bool ClientCaveTile::isLightState () const
{
	return _lightState;
}

inline void ClientCaveTile::setLightState (bool lightState)
{
	_lightState = lightState;
}
