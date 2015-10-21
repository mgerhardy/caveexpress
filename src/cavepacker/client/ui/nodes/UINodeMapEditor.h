#pragma once

#include "ui/nodes/IUINodeMapEditor.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/CavePackerAnimation.h"

namespace cavepacker {

class UINodeMapEditor: public IUINodeMapEditor {
public:
	UINodeMapEditor(IFrontend *frontend, IMapManager& mapManager) :
			IUINodeMapEditor(frontend, mapManager) {
	}

	virtual ~UINodeMapEditor() {
	}

	virtual IMapContext* getContext(const std::string& mapname) override {
		return new SokobanMapContext(mapname);
	}

	virtual const Animation& getEmitterAnimation(const EntityType& type) const {
		return Animation::NONE;
	}

	virtual void renderPlayer(int x, int y) const override {
		for (const IMap::StartPosition& position : _startPositions) {
			const SpriteDefPtr& def = SpriteDefinition::get().getFromEntityType(EntityTypes::PLAYER, Animation::NONE);
			const TileItem item = { this, def, &EntityTypes::PLAYER, 0, 0, string::toFloat(position._x), string::toFloat(position._y), LAYER_NONE, 0, "" };
			renderSprite(item, x, y);
		}
	}

	virtual MapEditorLayer getLayer (const SpriteType& type) const {
		if (SpriteTypes::isMapTile(type))
			return LAYER_BACKGROUND;
		if (SpriteTypes::isPackage(type))
			return LAYER_FOREGROUND;
		return LAYER_FOREGROUND;
	}

	virtual bool isMapTile(const SpriteType &type) const override {
		return SpriteTypes::isMapTile(type);
	}

	virtual bool isPlayer(const EntityType &type) const override {
		return EntityTypes::isPlayer(type);
	}
};

}
