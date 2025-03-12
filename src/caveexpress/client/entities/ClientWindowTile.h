#pragma once

#include "client/entities/ClientMapTile.h"

namespace caveexpress {

class ClientWindowTile: public ClientMapTile {
private:
	ClientWindowTile (uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align);

	bool _lightState;
	std::string _sprite;

public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const override;
	};
	static Factory FACTORY;

	virtual ~ClientWindowTile ();

	void setLightState (bool lightState);
	bool isLightState () const;

	void render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY, int mapPixelWidth, int mapPixelHeight) const override;
};

inline bool ClientWindowTile::isLightState () const
{
	return _lightState;
}

inline void ClientWindowTile::setLightState (bool lightState)
{
	_lightState = lightState;
}

}
