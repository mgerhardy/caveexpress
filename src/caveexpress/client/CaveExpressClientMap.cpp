#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "engine/client/particles/Bubble.h"
#include "engine/client/particles/Snow.h"
#include "engine/client/particles/Sparkle.h"
#include "engine/common/MapSettings.h"
#include "engine/common/network/messages/StopMovementMessage.h"
#include "engine/common/network/messages/MovementMessage.h"
#include "engine/common/network/messages/FingerMovementMessage.h"
#include "engine/common/network/messages/ClientInitMessage.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/client/ui/UI.h"
#include "engine/common/IFrontend.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/client/sound/Sound.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/EventHandler.h"
#include "engine/common/Logger.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/GLShared.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/DateUtil.h"
#include <SDL.h>

CaveExpressClientMap::CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth), _waterHeight(0.0)
{
}

void CaveExpressClientMap::resetCurrentMap ()
{
	ClientMap::resetCurrentMap();
	_waterHeight = 0.0f;
}

void CaveExpressClientMap::renderWater (int x, int y) const
{
	const float waterHeight = getWaterHeight();
	if (waterHeight <= 0.000001f)
		return;
	x += _screenRumbleOffsetX;
	y += _screenRumbleOffsetY;
	const int waterHeightPixel = waterHeight * _scale;

	const int mapPosY = y + _y;
	const int yWater = mapPosY + waterHeightPixel;
	const int widthWater = _frontend->getWidth();
	const Color waterLineColor = { 0.99f, 0.99f, 1.0f, 1.0f };
	static const Color color = { WATERCOLOR[0] / 255.0f, WATERCOLOR[1] / 255.0f, WATERCOLOR[2] / 255.0f, WATER_ALPHA
			/ 255.0f };
	_frontend->renderLine(x + _x, yWater - 1, x + _x + widthWater, yWater - 1, waterLineColor);
	const int xWater = x + _x;
	const int heightWater = _frontend->getHeight() - yWater;
	_frontend->renderFilledRect(xWater, yWater, widthWater, heightWater, color);
}

bool CaveExpressClientMap::drop ()
{
	if (isPause() || !isActive())
		return false;

	if (!_player || !_player->hasCollected())
		return false;

	// If the player has collected something, this will inform the server that he now wants to drop it
	const DropMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);

	return true;
}

void CaveExpressClientMap::setCaveState (uint16_t id, bool state)
{
	ClientEntityPtr e = getEntity(id);
	if (!e) {
		error(LOG_CLIENT, String::format("no entity with the id %i found in setCaveState", id));
		return;
	}

	if (EntityTypes::isWindow(e->getType())) {
		ClientWindowTile *tile = static_cast<ClientWindowTile*>(e.get());
		tile->setLightState(state);
	} else if (EntityTypes::isCave(e->getType())) {
		ClientCaveTile *tile = static_cast<ClientCaveTile*>(e.get());
		tile->setLightState(state);
	}
}

void CaveExpressClientMap::couldNotFindEntity (const std::string& prefix, uint16_t id) const
{
	ClientMap::couldNotFindEntity(prefix, id);
	for (ClientEntityMapConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const ClientEntityPtr e = i->second;
		if (EntityTypes::isMapTile(e->getType()))
			continue;
		info(LOG_CLIENT, String::format("id: %i, type: %s", e->getID(), e->getType().name.c_str()));
	}
}

void CaveExpressClientMap::init (uint16_t playerID) {
	ClientMap::init(playerID);
	// TODO: also take the non water height into account - so not have the amount of bubbles
	// on a small area when the water is rising
	const int bubbles = getWidth() / 100;
	for (int i = 0; i < bubbles; ++i) {
		_particleSystem.spawn(ParticlePtr(new Bubble(*this)));
	}

	const bool xmas = dateutil::isXmas();
	if (xmas || ThemeTypes::isIce(*_theme)) {
		// TODO: also take the non water height into account - so not have the amount of flakes
		// on a small area when the water is rising
		const int snowFlakes = getWidth() / 10;
		for (int i = 0; i < snowFlakes; ++i) {
			_particleSystem.spawn(ParticlePtr(new Snow(*this)));
		}
	}
}

void CaveExpressClientMap::start () {
	ClientMap::start();
	for (ClientEntityMapConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const ClientEntityPtr& e = i->second;
		if (!EntityTypes::isLava(e->getType())) {
			continue;
		}
		int startX, startY, sizeW, sizeH;
		e->getScreenPos(startX, startY);
		e->getScreenSize(sizeW, sizeH);
		const int border = 5;
		sizeW -= border;
		startX += border;
		startY += sizeH / 2.0f;
		const int sparklePerLava = 4;
		for (int p = 0; p < sparklePerLava; ++p) {
			_particleSystem.spawn(ParticlePtr(new Sparkle(*this, startX, startY, sizeW, sizeH)));
		}
	}
}

void CaveExpressClientMap::render (int x, int y) const
{
	ExecutionTime renderTime("ClientMapRender", 2000L);
	const int baseX = x + _x + _camera.getViewportX();
	const int baseY = y + _y + _camera.getViewportY();
	int layerX = baseX;
	int layerY = baseY;
	getLayerOffset(layerX, layerY);

	_frontend->enableScissor(layerX, layerY,
			std::min(getWidth() - layerX, _mapWidth * _scale),
			std::min(getHeight() - layerY, _mapHeight * _scale));
	renderLayer(layerX, layerY, LAYER_BACK);
	renderLayer(layerX, layerY, LAYER_MIDDLE);
	renderLayer(layerX, layerY, LAYER_FRONT);

	if (_restartDue != 0) {
		renderFadeOutOverlay(x, y);
	}

	Config.setDebugRendererData(layerX, layerY, getWidth(), getHeight(), _scale);
	Config.getDebugRenderer().render();

	renderWater(x + _x, layerY);

	_particleSystem.render(_frontend, layerX, layerY);

	_frontend->disableScissor();
}
