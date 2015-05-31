#include "UINodeBackgroundScene.h"
#include "client/ui/UI.h"
#include "client/ClientMap.h"
#include "caveexpress/shared/CaveExpressMapFailedReasons.h"

UINodeBackgroundScene::UINodeBackgroundScene (IFrontend *frontend) :
		UINode(frontend), _reason(&MapFailedReasons::FAILED_NO), _theme(&ThemeTypes::ROCK)
{
	setPadding(getScreenPadding());
	setSize(1.0, 1.0f);
	setBackgroundColor(colorBlack);
	_caveOffIce = loadTexture("ui-scene-cave-ice");
	_caveArtIce = loadTexture("ui-scene-caveart-ice");
	_caveOffRock = loadTexture("ui-scene-cave-rock");
	_caveArtRock = loadTexture("ui-scene-caveart-rock");
	_imageWidth = _caveOffIce->getWidth();
	_imageHeight = _caveOffIce->getHeight();
	_amountHorizontal = getRenderWidth(false) / _imageWidth + 1;
	_amountVertical = getRenderHeight(false) / _imageHeight + 1;

	_tilesIce.push_back(loadTexture("ui-scene-tile1-ice"));
	_tilesIce.push_back(loadTexture("ui-scene-tile2-ice"));
	_groundsIce.push_back(loadTexture("ui-scene-ground1-ice"));
	_groundsIce.push_back(loadTexture("ui-scene-ground2-ice"));
	_tilesRock.push_back(loadTexture("ui-scene-tile1-rock"));
	_tilesRock.push_back(loadTexture("ui-scene-tile2-rock"));
	_groundsRock.push_back(loadTexture("ui-scene-ground1-rock"));
	_groundsRock.push_back(loadTexture("ui-scene-ground2-rock"));

	_failed[&MapFailedReasons::FAILED_WATER_HEIGHT] = loadTexture("dead-waterheight");
	_failed[&MapFailedReasons::FAILED_HITPOINTS] = loadTexture("dead-hitpoints");
	_failed[&MapFailedReasons::FAILED_SIDESCROLL] = loadTexture("dead-hitpoints");
	_failed[&MapFailedReasons::FAILED_NPC_FLYING] = loadTexture("dead-npc-flying");
	_failed[&MapFailedReasons::FAILED_NPC_WALKING] = loadTexture("dead-npc-walking");
	_failed[&MapFailedReasons::FAILED_NPC_FISH] = loadTexture("dead-npc-fish");
	_failed[&MapFailedReasons::FAILED_NPC_MAMMUT] = loadTexture("dead-npc-mammut");
	_failed[&MapFailedReasons::FAILED_NO_MORE_PLAYERS] = loadTexture("dead-hitpoints");
}

void UINodeBackgroundScene::updateReason (const MapFailedReason& reason, const ThemeType& theme)
{
	_reason = &reason;
	_theme = &theme;
}

void UINodeBackgroundScene::renderBackground (int x, int y) const
{
	const TextureVector& tiles = ThemeTypes::isIce(*_theme) ? _tilesIce : _tilesRock;
	const int tileCnt = tiles.size();
	const int renderHeight = getRenderHeight(false);
	for (int row = 0; row <= _amountVertical; ++row) {
		const int renderY = y + renderHeight - (row * _imageHeight + _imageHeight / 4);
		for (int col = 0; col <= _amountHorizontal; ++col) {
			const TexturePtr& t = tiles[((row * _amountHorizontal) + col) % tileCnt];
			renderImage(t, x + _imageWidth * col, renderY);
		}
	}
}

void UINodeBackgroundScene::renderCave (int x, int y) const
{
	const int renderHeight = getRenderHeight(false);
	if (ThemeTypes::isIce(*_theme)) {
		renderImage(_caveOffIce, x, y + renderHeight - _imageHeight / 4);
		renderImage(_caveArtIce, x, y + renderHeight - _imageHeight - _imageHeight / 4);
	} else {
		renderImage(_caveOffRock, x, y + renderHeight - _imageHeight / 4);
		renderImage(_caveArtRock, x, y + renderHeight - _imageHeight - _imageHeight / 4);
	}
}

int UINodeBackgroundScene::renderGround (int x, int y) const
{
	const TextureVector& tiles = ThemeTypes::isIce(*_theme) ? _groundsIce : _groundsRock;
	const int groundCnt = tiles.size();
	const int renderHeight = getRenderHeight(false);
	const int groundY = y + renderHeight - _imageHeight / 4;
	for (int col = 0; col <= _amountHorizontal; ++col) {
		const TexturePtr& t = tiles[col % groundCnt];
		renderImage(t, x + _imageWidth * col, groundY);
	}
	return groundY;
}

void UINodeBackgroundScene::renderWater (int x, int y) const
{
	static const Color color = { WATERCOLOR[0] / 255.0f, WATERCOLOR[1] / 255.0f, WATERCOLOR[2] / 255.0f, WATER_ALPHA
			/ 255.0f };
	_frontend->renderFilledRect(x, y, _frontend->getWidth() - x, _frontend->getHeight() - y, color);
}

void UINodeBackgroundScene::renderFailedOnGround (int x, int y, const MapFailedReason& reason, float offsetY) const
{
	renderCave(x, y);
	const int groundY = renderGround(x, y);
	FailedMap::const_iterator i = _failed.find(&reason);
	if (i == _failed.end())
		return;
	const TexturePtr& t = i->second;
	if (!t || !t->isValid())
		return;

	const int playerX = x + getRenderCenterX() - t->getWidth() / 2;
	const int playerY = groundY - t->getHeight() + offsetY * t->getHeight();
	renderImage(t, playerX, playerY);
}

void UINodeBackgroundScene::renderFailedCenter (int x, int y, const MapFailedReason& reason) const
{
	FailedMap::const_iterator i = _failed.find(&reason);
	if (i == _failed.end())
		return;
	const TexturePtr& t = i->second;
	if (!t || !t->isValid())
		return;

	const int playerX = x + getRenderCenterX() - t->getWidth() / 2;
	const int playerY = y + getRenderCenterY() - t->getHeight() / 2;
	renderImage(t, playerX, playerY);
}

void UINodeBackgroundScene::renderFailedNpcFlying (int x, int y) const
{
	const float offsetY = 0.046831956f;
	renderFailedOnGround(x, y, MapFailedReasons::FAILED_NPC_FLYING, offsetY);
}

void UINodeBackgroundScene::renderFailedNpcWalking (int x, int y) const
{
	const float offsetY = 0.104895105f;
	renderFailedOnGround(x, y, MapFailedReasons::FAILED_NPC_WALKING, offsetY);
}

void UINodeBackgroundScene::renderFailedNpcMammut (int x, int y) const
{
	const float offsetY = 0.138810198f;
	renderFailedOnGround(x, y, MapFailedReasons::FAILED_NPC_MAMMUT, offsetY);
}

void UINodeBackgroundScene::renderFailedHitpoints (int x, int y) const
{
	const float offsetY = 0.190821256f;
	renderFailedOnGround(x, y, MapFailedReasons::FAILED_HITPOINTS, offsetY);
}

void UINodeBackgroundScene::renderFailedNpcFish (int x, int y) const
{
	renderFailedCenter(x, y, MapFailedReasons::FAILED_NPC_FISH);
	renderWater(x, y);
}

void UINodeBackgroundScene::renderFailedWaterHeight (int x, int y) const
{
	renderCave(x, y);
	renderGround(x, y);
	renderFailedCenter(x, y, MapFailedReasons::FAILED_WATER_HEIGHT);
	renderWater(x, y);
}

void UINodeBackgroundScene::render (int x, int y) const
{
	UINode::render(x, y);

	x += getRenderX(false);
	y += getRenderY(false);

	renderBackground(x, y);

	if (MapFailedReasons::FAILED_WATER_HEIGHT == *_reason)
		renderFailedWaterHeight(x, y);
	else if (MapFailedReasons::FAILED_NPC_FLYING == *_reason)
		renderFailedNpcFlying(x, y);
	else if (MapFailedReasons::FAILED_NPC_WALKING == *_reason)
		renderFailedNpcWalking(x, y);
	else if (MapFailedReasons::FAILED_NPC_MAMMUT == *_reason)
		renderFailedNpcMammut(x, y);
	else if (MapFailedReasons::FAILED_NPC_FISH == *_reason)
		renderFailedNpcFish(x, y);
	else if (MapFailedReasons::FAILED_HITPOINTS == *_reason || MapFailedReasons::FAILED_SIDESCROLL == *_reason)
		renderFailedHitpoints(x, y);
}
