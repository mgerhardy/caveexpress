#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressCooldown.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "common/vec2.h"
#include "particles/Bubble.h"
#include "particles/Snow.h"
#include "particles/Rain.h"
#include "particles/Sparkle.h"
#include "common/MapSettings.h"
#include "network/messages/StopMovementMessage.h"
#include "network/messages/MovementMessage.h"
#include "network/messages/FingerMovementMessage.h"
#include "network/messages/ClientInitMessage.h"
#include "client/entities/ClientMapTile.h"
#include "ui/UI.h"
#include "common/IFrontend.h"
#include "network/ProtocolHandlerRegistry.h"
#include "sound/Sound.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "common/Log.h"
#include "service/ServiceProvider.h"
#include "common/ExecutionTime.h"
#include "common/DateUtil.h"
#include <SDL.h>
#include <SDL_image.h>

namespace caveexpress {

CaveExpressClientMap::CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth)
{
}

void CaveExpressClientMap::resetCurrentMap ()
{
	ClientMap::resetCurrentMap();
	_waterHeight = 0.0f;
}

SDL_Rect CaveExpressClientMap::getWaterRect(int x, int y) const {
	const int waterWidth = std::min(_width, static_cast<int>(getPixelWidth() * _zoom + std::min(x, 0))) - 1;
	const int waterGround = std::min(_height, y + static_cast<int>(getPixelHeight() * _zoom));
	const int waterSurface = y + getWaterSurface() * _zoom;
	const int waterHeight = waterGround - waterSurface;
	return SDL_Rect{std::max(0, x), waterSurface, waterWidth, waterHeight};
}

void CaveExpressClientMap::renderWater (int x, int y) const
{
	if (getWaterHeight() <= 0.000001f)
		return;
	const SDL_Rect& rect = getWaterRect(x, y);
	Log::trace(LOG_GAMEIMPL, "rect:(%i,%i,%i,%i), x:%i, y:%i, water:(w:%i, h:%i, surf:%i, grnd:%i, wh:%f, scale:%i)",
									_x, _y, _width, _height, x, y, rect.w, rect.h, rect.y, rect.y + rect.h, _waterHeight, _scaleGridToPixel);
	vec2 offsets = vec2_zero; // TODO: implement me - see https://github.com/mgerhardy/caveexpress/issues/171
	_frontend->renderWaterPlane(rect.x, rect.y, rect.w, rect.h, waterColor, waterLineColor, offsets);
	if (Config.isDebug()) {
		const int waterGround = rect.y + rect.h;
		_frontend->renderLine(rect.x, rect.y, rect.x + rect.w, rect.y, colorRed);
		_frontend->renderLine(rect.x, waterGround - 1, rect.x + rect.w, waterGround - 1, colorGreen);
		_frontend->renderLine(rect.x, rect.y, rect.x, waterGround, colorRed);
		_frontend->renderLine(rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, waterGround, colorGreen);
	}
}

bool CaveExpressClientMap::drop ()
{
	if (isPause() || !isActive())
		return false;

	if (!_player || !_player->hasCollected())
		return false;

	// If the player has collected something, this will inform the server that he now wants to drop it
	static const DropMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);

	return true;
}

void CaveExpressClientMap::calcCaveSignOffset(const ClientEntityPtr &e, const SpritePtr &caveSignSprite, const SpritePtr &caveSprite,
									 int &offsetX, int &offsetY) {
	switch (e->getAlignment()) {
	case ENTITY_ALIGN_UPPER_LEFT:
		break;
	case ENTITY_ALIGN_LOWER_LEFT:
		offsetX = (int)((float)caveSprite->getMaxWidth() / 2.0f);
		offsetY = (int)((float)(caveSprite->getMaxHeight() - caveSignSprite->getMaxHeight()));
		break;
	case ENTITY_ALIGN_MIDDLE_CENTER:
		offsetX = (int)((float)(caveSprite->getMaxWidth() - caveSignSprite->getMaxWidth()) / 2.0f);
		offsetY = (int)((float)(caveSprite->getMaxHeight() - caveSignSprite->getMaxHeight()) / 2.0f);
		break;
	}
}

void CaveExpressClientMap::setCaveNumber(uint16_t id, uint8_t number) {
	if (number == 0)
		return;
	Log::debug(LOG_GAMEIMPL, "set cave for %i to %i", id, number);
	ClientEntityPtr e = getEntity(id);
	if (!e) {
		Log::error(LOG_GAMEIMPL, "no cave entity with the id %i found", id);
		return;
	}
	const char first = (char)(number / 10 + '0');
	const char second = (char)(number % 10 + '0');
	const std::string caveSignSpriteName = string::format("cave-sign-%c%c", first, second);
	const SpritePtr &caveSignSprite = UI::get().loadSprite(caveSignSpriteName);
	if (!caveSignSprite) {
		Log::error(LOG_GAMEIMPL, "no sprite found for %s", caveSignSpriteName.c_str());
		return;
	}
	const SpritePtr &caveSprite = e->getSprite();
	if (!caveSprite) {
		Log::error(LOG_GAMEIMPL, "no sprite found for %s", e->getSpriteName().c_str());
		return;
	}
	int offsetX = 0;
	int offsetY = 0;
	calcCaveSignOffset(e, caveSignSprite, caveSprite, offsetX, offsetY);
	e->addOverlay(caveSignSprite, offsetX, offsetY);
}

void CaveExpressClientMap::setCaveState (uint16_t id, bool state)
{
	ClientEntityPtr e = getEntity(id);
	if (!e) {
		Log::error(LOG_GAMEIMPL, "no entity with the id %i found in setCaveState", id);
		return;
	}

	if (EntityTypes::isWindow(e->getType())) {
		ClientWindowTile *tile = static_cast<ClientWindowTile*>(e);
		tile->setLightState(state);
	} else if (EntityTypes::isCave(e->getType())) {
		ClientCaveTile *tile = static_cast<ClientCaveTile*>(e);
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
		Log::info(LOG_GAMEIMPL, "id: %i, type: %s", e->getID(), e->getType().name.c_str());
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
	} else if (ThemeTypes::isJungle(*_theme)) {
		const int rainDrops = int(randBetweenf(1.2f, 4.5f) * (float)getMapWidth() * 100);
		//Log::info(LOG_GAMEIMPL, "RAIN drops: %i", rainDrops);
		for (int i = 0; i < rainDrops; ++i) {
			_particleSystem.spawn(ParticlePtr(new Rain(*this)));
		}
	}
	for (auto iter = ThemeType::begin(); iter != ThemeType::end(); ++iter) {
		Log::debug(LOG_GAMEIMPL, "Registered theme types: %s", iter->second->name.c_str());
	}

#if 0
	// write all sprites as png files with SDL_image to the harddisk
	for (const auto &iter : _serviceProvider.getTextureDefinition().getMap()) {
		const TextureDef &textureDefinition = iter.second;
		if (textureDefinition.id.find("-right-") != std::string::npos) {
			continue;
		}
		const std::string &atlasFilename = string::format("base/caveexpress/pics/%s.png", textureDefinition.textureName.c_str());
		SDL_RWops *atlasRW = SDL_RWFromFile(atlasFilename.c_str(), "rb");
		if (!atlasRW) {
			Log::error(LOG_GAMEIMPL, "Failed to load %s", atlasFilename.c_str());
			continue;
		}
		SDL_Surface *textureAtlasSurface = IMG_LoadPNG_RW(atlasRW);
		if (!textureAtlasSurface) {
			Log::error(LOG_GAMEIMPL, "Failed to load %s", atlasFilename.c_str());
			SDL_RWclose(atlasRW);
			continue;
		}
		const TextureDefinitionTrim trim = textureDefinition.trim;
		const int w = trim.untrimmedWidth;
		const int h = trim.untrimmedHeight;
		const int x = trim.trimmedOffsetX;
		const int y = trim.trimmedOffsetY;
		const int width = trim.trimmedWidth;
		const int height = trim.trimmedHeight;
		const TextureDefinitionCoords &texcoords = textureDefinition.texcoords;
		const int x0 = (int)(texcoords.x0 * (float)textureAtlasSurface->w);
		const int y0 = (int)(texcoords.y0 * (float)textureAtlasSurface->h);
		const int x1 = (int)(texcoords.x1 * (float)textureAtlasSurface->w);
		const int y1 = (int)(texcoords.y1 * (float)textureAtlasSurface->h);
		const SDL_Rect rect = {x0, y0, x1, y1};
		SDL_Rect destRect {x, y, width, height};
		SDL_Surface *sprite = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
		SDL_BlitSurface(textureAtlasSurface, &rect, sprite, &destRect);
		printf("load atlas %s\n", atlasFilename.c_str());
		printf("Save sprite %s\n", textureDefinition.id.c_str());
		std::string spriteFilename = string::format("%s.png", textureDefinition.id.c_str());
		if (spriteFilename.find("-left-") != std::string::npos) {
			spriteFilename.replace(spriteFilename.find("-left-"), 6, "-DIR-");
		}
		IMG_SavePNG(sprite, spriteFilename.c_str());
		SDL_FreeSurface(sprite);
		SDL_FreeSurface(textureAtlasSurface);
		SDL_RWclose(atlasRW);
	}
#endif
	_camera.update(vec2_zero, 0, _zoom);
}

void CaveExpressClientMap::renderBegin (int x, int y) const
{
	_target = _frontend->renderToTexture(_x, _y, _width, _height);
	ClientMap::renderBegin(x, y);
}

void CaveExpressClientMap::renderEnd (int x, int y) const
{
	ClientMap::renderEnd(x, y);
	if (_target)
		_frontend->renderTarget(_target);
	renderWater(x, y);
}

int CaveExpressClientMap::renderCooldownDescription (uint32_t cooldownIndex, int x, int y, int w, int h) const
{
	ClientMap::renderCooldownDescription(cooldownIndex, x, y, w, h);
	const int padding = 5;
	if (Cooldowns::INVULNERABLE.id == cooldownIndex) {
		const std::string& text = tr("Invulnerable");
		_font->print(text, colorWhite, x + w + padding, y);
		return 2 * padding + _font->getTextWidth(text);
	}
	return 0;
}

void CaveExpressClientMap::setSetting (const std::string& key, const std::string& value)
{
	Super::setSetting(key, value);
	if (key == msn::WIND) {
		_wind = string::toFloat(value);
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
		startY += (int)((float)sizeH / 2.0f);
		const int sparklePerLava = 4;
		for (int p = 0; p < sparklePerLava; ++p) {
			_particleSystem.spawn(ParticlePtr(new Sparkle(*this, startX, startY, sizeW, sizeH)));
		}
	}
}

}
