#include "UINodeMapEditor.h"
#include "common/FileSystem.h"
#include "common/KeyValueParser.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/constants/EmitterSettings.h"
#include "caveexpress/server/map/RandomMapContext.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/constants/Commands.h"

namespace caveexpress {

UINodeMapEditor::UINodeMapEditor (IFrontend *frontend, IMapManager& mapManager) :
		IUINodeMapEditor(frontend, mapManager), _waterHeight(0.0f) {
}

UINodeMapEditor::~UINodeMapEditor ()
{
}

const Animation& UINodeMapEditor::getEmitterAnimation (const EntityType& type) const
{
	return EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
}

bool UINodeMapEditor::isMapTile (const SpriteType &type) const {
	return SpriteTypes::isMapTile(type);
}

bool UINodeMapEditor::isPlayer (const EntityType &type) const {
	return EntityTypes::isPlayer(type);
}

IMapContext* UINodeMapEditor::getContext (const std::string& mapname) {
	return new CaveExpressMapContext(mapname);
}

MapEditorLayer UINodeMapEditor::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isRock(type) || SpriteTypes::isAnyGround(type) || SpriteTypes::isWaterFall(type)) {
		return LAYER_SOLID;
	} else if (SpriteTypes::isBackground(type) || SpriteTypes::isWindow(type) || SpriteTypes::isCave(type)) {
		return LAYER_BACKGROUND;
	} else if (SpriteTypes::isBridge(type)) {
		return LAYER_FOREGROUND;
	} else if (SpriteTypes::isLiane(type)) {
		return LAYER_DECORATION;
	}
	return LAYER_SOLID;
}

void UINodeMapEditor::renderPlayer (int x, int y) const
{
	for (const IMap::StartPosition& position : _startPositions) {
		const SpriteDefPtr& def = SpriteDefinition::get().getFromEntityType(EntityTypes::PLAYER, Animations::ANIMATION_IDLE);
		const TileItem item = { this, def, &EntityTypes::PLAYER, 0, 0, string::toFloat(position._x), string::toFloat(position._y), LAYER_NONE, 0, "" };
		renderSprite(item, x, y);
	}
	renderWater(x, y);
}

void UINodeMapEditor::renderHighlightItem (int x, int y) const
{
	if (_highlightItem == nullptr)
		return;

	const SpriteType& type = _highlightItem->def->type;
	std::string str;
	if (SpriteTypes::isCave(type)) {
		str += _highlightItem->entityType->name;
		str += " with delay: ";
		str += string::toString(_highlightItem->delay);
	} else if (_highlightItem->entityType != nullptr) {
		if (EntityTypes::isEmitter(*_highlightItem->entityType)) {
			str += _highlightItem->entityType->name;
			str += " with delay: ";
			str += string::toString(_highlightItem->delay);
		}
	}

	if (str.empty())
		return;

	x += getRenderX() + _highlightItem->gridX * getTileWidth();
	y += getRenderY() + _highlightItem->gridY * getTileHeight();
	renderFilledRect(x, y, _font->getTextWidth(str), _font->getTextHeight(str), colorWhite);
	_font->print(str, _fontColor, x, y);
}

void UINodeMapEditor::renderWater (int x, int y) const
{
	const float waterY = _mapHeight - _gridScrollY - _waterHeight;
	const float tileHeight = getTileHeight();
	x += getRenderX();
	y += getRenderY();
	const int waterPixelSurface = y + waterY * tileHeight;
	if (waterPixelSurface > y + getRenderHeight())
		return;

	const float tileWidth = getTileWidth();
	const int waterPixelHeight = _waterHeight * tileHeight;
	const int waterPixelWidth = getScreenMapGridWidth() * tileWidth;

	Color waterColor = { 0.7f, 0.7f, 1.0f, 0.8f };
	renderLine(x, waterPixelSurface, x + waterPixelWidth, waterPixelSurface, waterColor);
	waterColor[3] = 0.4;
	renderFilledRect(x, waterPixelSurface + 1, waterPixelWidth, waterPixelHeight, waterColor);
}

bool UINodeMapEditor::placeTileItem (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
		MapEditorLayer layer, bool overwrite, EntityAngle angle)
{
	const SpriteType& type = def->type;
	if (SpriteTypes::isCave(type))
		return placeCave(def, &EntityType::NONE, gridX, gridY, layer, -1, overwrite);
	const TileItem item = { this, def, entityType, 0, 0, gridX, gridY, layer, angle, "" };
	return IUINodeMapEditor::placeTileItem(item, overwrite);
}

bool UINodeMapEditor::placeEmitter (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
		int emitterAmount, int emitterDelay, bool overwrite, EntityAngle angle, const std::string& settings) {
	if (_activeEntityType != nullptr && EntityTypes::isPlayer(*_activeEntityType)) {
		setPlayerPosition(gridX, gridY);
		return true;
	}
	std::string str = settings;
	if (_activeEntityType != nullptr && EntityTypes::hasDirection(*_activeEntityType) && !_activeEntityTypeRight) {
		str = EMITTER_RIGHT "=false";
	}
	return IUINodeMapEditor::placeEmitter(def, entityType, gridX, gridY, emitterAmount, emitterDelay, overwrite, angle, str);
}

bool UINodeMapEditor::placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX,
		gridCoord gridY, MapEditorLayer layer, int delay, bool overwrite)
{
	const TileItem item = { this, def, entityType, 0, delay, gridX, gridY, layer, 0, "" };
	const bool ret = IUINodeMapEditor::placeTileItem(item, overwrite);
	if (ret) {
		notifyTilePlaced(def);
	}
	return ret;
}

bool UINodeMapEditor::isOverlapping (const TileItem& item1, const TileItem& item2) const
{
	switch (item1.layer) {
	case LAYER_BACKGROUND:
		if (item2.layer == LAYER_FOREGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER) {
			return false;
		}
		break;
	case LAYER_FOREGROUND:
		if (item2.layer == LAYER_BACKGROUND || item2.layer == LAYER_DECORATION || item2.layer == LAYER_EMITTER) {
			return false;
		}
		break;
	case LAYER_DECORATION:
	case LAYER_EMITTER:
		if (item2.entityType != nullptr && EntityTypes::isNpc(*item2.entityType))
			break;
		if (item2.layer != LAYER_SOLID) {
			return false;
		}
		break;
	case LAYER_SOLID:
	case LAYER_NONE:
	case LAYER_MAX:
		break;
	}
	const bool useShape = true;
	const vec2& size = item1.getSize(useShape);
	const gridCoord x = item1.gridX + item1.getX(useShape) + EPSILON;
	const gridCoord y = item1.gridY + item1.getY(useShape) + EPSILON;
	const gridSize w = size.x - 2.0f * EPSILON;
	const gridSize h = size.y - 2.0f * EPSILON;
	return IUINodeMapEditor::isOverlapping(x, y, w, h, item2);
}

void UINodeMapEditor::setState (const State& state)
{
	IUINodeMapEditor::setState(state);
	setWaterHeight(string::toFloat(_settings[msn::WATER_HEIGHT]));
}

void UINodeMapEditor::doClear ()
{
	IUINodeMapEditor::doClear();
	setPackageTransferCount(string::toInt(msd::PACKAGE_TRANSFER_COUNT));
	setFlyingNpc(string::toBool(msd::FLYING_NPC));
	setFishNpc(string::toBool(msd::FISH_NPC));
	setWaterParameters(string::toFloat(msd::WATER_HEIGHT), string::toFloat(msd::WATER_CHANGE), string::toInt(msd::WATER_RISING_DELAY), string::toInt(msd::WATER_FALLING_DELAY));
	setSetting(msn::SIDEBORDERFAIL, msd::SIDEBORDERFAIL);
	setSetting(msn::WIND, msd::WIND);
	setSetting(msn::POINTS, string::toString(msdv::POINTS));
	setSetting(msn::REFERENCETIME, string::toString(msdv::REFERENCETIME));
	setGravity(msdv::GRAVITY);
}

bool UINodeMapEditor::shouldSaveTile (const TileItem& tile) const
{
	const SpriteType& spriteType = tile.def->type;
	return tile.entityType == nullptr && !SpriteTypes::isCave(spriteType);
}

bool UINodeMapEditor::shouldSaveEmitter (const TileItem& tile) const
{
	const SpriteType& spriteType = tile.def->type;
	return tile.entityType != nullptr && !SpriteTypes::isCave(spriteType);
}

void UINodeMapEditor::saveTiles (const FilePtr& file, const TileItems& map) const
{
	IUINodeMapEditor::saveTiles(file, map);
	bool caveAdded = false;
	for (TileItemsConstIter i = map.begin(); i != map.end(); ++i) {
		if (i->gridX >= _mapWidth || i->gridY >= _mapHeight)
			continue;
		const SpriteType& spriteType = i->def->type;
		if (SpriteTypes::isCave(spriteType)) {
			file->appendString("\tmap:addCave(\"");
			file->appendString(i->def->id.c_str());
			file->appendString("\", ");
			file->appendString(string::toString(i->gridX).c_str());
			file->appendString(", ");
			file->appendString(string::toString(i->gridY).c_str());
			if (!i->entityType->isNone()) {
				file->appendString(", \"");
				file->appendString(i->entityType->name.c_str());
				file->appendString("\"");
			}
			if (i->delay > -1) {
				if (i->entityType->isNone()) {
					file->appendString(", \"");
					file->appendString(i->entityType->name.c_str());
					file->appendString("\"");
				}
				file->appendString(", ");
				file->appendString(string::toString(i->delay).c_str());
			}
			file->appendString(")\n");
			caveAdded = true;
		}
	}

	if (caveAdded)
		file->appendString("\n");
}

void UINodeMapEditor::loadFromContext (IMapContext& ctx)
{
	ctx.load(true);
	setFileName(ctx.getName());
	setMapName(ctx.getTitle());
	_lastMap->setValue(ctx.getName());

	const IMap::SettingsMap& settings = ctx.getSettings();
	for (IMap::SettingsMapConstIter i = settings.begin(); i != settings.end(); ++i) {
		if (i->first == msn::WATER_HEIGHT)
			setWaterHeight(string::toFloat(i->second));
		else if (i->first == msn::THEME)
			setTheme(ThemeType::getByName(i->second));
		else
			setSetting(i->first, i->second);
	}
	_startPositions = ctx.getStartPositions();
	const int mapWidth = string::toInt(_settings[msn::WIDTH]);
	const int mapHeight = string::toInt(_settings[msn::HEIGHT]);
	setMapDimensions(mapWidth, mapHeight);

	const std::vector<MapTileDefinition>& mapTiles = ctx.getMapTileDefinitions();
	Log::info(LOG_UI, "place %i maptiles", static_cast<int>(mapTiles.size()));
	for (std::vector<MapTileDefinition>::const_iterator i = mapTiles.begin(); i != mapTiles.end(); ++i) {
		const SpriteType& type = i->spriteDef->type;
		const MapEditorLayer layer = getLayer(type);
		if (!placeTileItem(i->spriteDef, nullptr, i->x, i->y, layer, false, i->angle))
			Log::error(LOG_UI, "could not place tile %s at %f:%f", i->spriteDef->id.c_str(), i->x, i->y);
	}
	const std::vector<CaveTileDefinition>& caves = ((CaveExpressMapContext*)&ctx)->getCaveTileDefinitions();
	Log::info(LOG_UI, "place %i caves", static_cast<int>(caves.size()));
	for (std::vector<CaveTileDefinition>::const_iterator i = caves.begin(); i != caves.end(); ++i) {
		const SpriteType& type = i->spriteDef->type;
		const MapEditorLayer layer = getLayer(type);
		if (!placeCave(i->spriteDef, i->type, i->x, i->y, layer, i->delay, false))
			Log::error(LOG_UI, "could not place cave %s at %i:%i", i->spriteDef->id.c_str(), (int)i->x, (int)i->y);
	}
	const std::vector<EmitterDefinition>& emitters = ctx.getEmitterDefinitions();
	Log::info(LOG_UI, "place %i emitters", static_cast<int>(emitters.size()));
	for (std::vector<EmitterDefinition>::const_iterator i = emitters.begin(); i != emitters.end(); ++i) {
		const EntityType& entityType = *i->type;
		const gridCoord x = i->x;
		const gridCoord y = i->y;
		const int amount = i->amount;
		const int delay = i->delay;
		const KeyValueParser s(i->settings);
		const Animation& animation = EntityTypes::hasDirection(entityType) ?
			(s.getBool(EMITTER_RIGHT, true) ?
					Animations::ANIMATION_IDLE_RIGHT :
					Animations::ANIMATION_IDLE_LEFT) :
					Animations::ANIMATION_IDLE;
		const SpriteDefPtr def = SpriteDefinition::get().getFromEntityType(entityType, animation);
		if (!def) {
			Log::error(LOG_UI, "could not get the sprite definition for the entity type: %s", entityType.name.c_str());
			continue;
		}
		if (!placeEmitter(def, &entityType, x, y, amount, delay, false, s.getFloat(EMITTER_ANGLE), i->settings))
			Log::error(LOG_UI, "could not place emitter %s at %f:%f", i->type->name.c_str(), x, y);
	}
}

void UINodeMapEditor::autoFill (const ThemeType& theme)
{
	Undo();
	srand(time(nullptr));
	const int initialTiles = _mapWidth / 4;
	const int rockAmountFactor = 3 + rand() % 6;
	const int rockAmount = initialTiles * rockAmountFactor;
	const std::string name = "editor-random-" + theme.name;
	RandomMapContext ctx(name, theme, initialTiles, rockAmount, _mapWidth, _mapHeight);
	ctx.setSettings(_settings);
	const IMap::SettingsMap settings = _settings;
	const std::string oldName = getName();
	loadFromContext(ctx);
	setFileName(oldName);
}

}
