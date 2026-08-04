#pragma once

#include "client/entities/ClientMapTile.h"

namespace caveexpress {

class ClientGate: public ClientMapTile {
private:
	ClientGate (uint16_t id, const std::string& sprite, const Animation& animation, float x,
			float y, float sizeX, float sizeY, const SoundMapping& soundMapping, EntityAlignment align);

	uint8_t _openAmount;

	void applyOpenFrame () const;

public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const override;
	};
	static Factory FACTORY;

	virtual ~ClientGate ();

	void setOpenAmount (uint8_t openAmount);
	uint8_t getOpenAmount () const { return _openAmount; }

	bool update (uint32_t deltaTime, bool lerpPos, bool animateSpriteAlways = true) override;
	void render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY, int mapPixelWidth, int mapPixelHeight) const override;
};

}
