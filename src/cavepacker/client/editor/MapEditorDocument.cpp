#include "MapEditorDocument.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/CavePackerAnimation.h"

namespace cavepacker {

MapEditorDocument::MapEditorDocument (IMapManager& mapManager) :
		IMapEditorDocument(mapManager)
{
}

MapEditorLayer MapEditorDocument::getLayer (const SpriteType& type) const
{
	if (SpriteTypes::isMapTile(type))
		return LAYER_BACKGROUND;
	if (SpriteTypes::isPackage(type))
		return LAYER_FOREGROUND;
	return LAYER_FOREGROUND;
}

bool MapEditorDocument::isMapTileType (const SpriteType& type) const
{
	return SpriteTypes::isMapTile(type);
}

bool MapEditorDocument::isPlayerType (const EntityType& type) const
{
	return EntityTypes::isPlayer(type);
}

const Animation& MapEditorDocument::getEmitterAnimation (const EntityType& type) const
{
	return Animation::NONE;
}

const EntityType& MapEditorDocument::getPlayerEntityType () const
{
	return EntityTypes::PLAYER;
}

std::unique_ptr<IMapContext> MapEditorDocument::createContext (const std::string& mapName) const
{
	return std::unique_ptr<IMapContext>(new SokobanMapContext(mapName));
}

void MapEditorDocument::fillTilePalette (std::vector<SpriteDefPtr>& out) const
{
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& sprite = i->second;
		if (!SpriteTypes::isMapTile(sprite->type) && !SpriteTypes::isPackage(sprite->type))
			continue;
		if (sprite->hasNoTextures())
			continue;
		out.push_back(sprite);
	}
}

void MapEditorDocument::fillEntityPalette (std::vector<const EntityType*>& out) const
{
	for (auto i = EntityType::begin(); i != EntityType::end(); ++i) {
		if (EntityTypes::isDynamic(*i->second))
			out.push_back(i->second);
	}
}

bool MapEditorDocument::placeBrushItem (bool overwrite)
{
	if (!_activeSprite)
		return false;
	if (_activeEntityType != nullptr && isPlayerType(*_activeEntityType)) {
		setPlayerPosition(_selectedGridX, _selectedGridY);
		return true;
	}

	MapEditorTileItem item;
	item.gridX = _selectedGridX;
	item.gridY = _selectedGridY;
	item.angle = _activeAngle;
	item.entityType = nullptr;

	if (_activeEntityType != nullptr && EntityTypes::isPackage(*_activeEntityType)) {
		item.def = SpriteDefinition::get().getSpriteDefinition("package");
		if (!item.def)
			item.def = _activeSprite;
	} else {
		item.def = _activeSprite;
	}

	item.layer = getLayer(item.def->type);
	item.mapTile = true;
	if (!canPlaceTileItem(item))
		return false;
	return placeTileItem(item, overwrite);
}

}
