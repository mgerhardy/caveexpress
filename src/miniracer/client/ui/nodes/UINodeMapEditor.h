#pragma once

#include "ui/nodes/IUINodeMapEditor.h"
#include "miniracer/server/map/MiniRacerMapContext.h"
#include "miniracer/shared/MiniRacerSpriteType.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include "miniracer/shared/MiniRacerAnimation.h"
#include "miniracer/server/map/MiniRacerMapContext.h"

namespace miniracer {

class UINodeMapEditor: public IUINodeMapEditor {
public:
	UINodeMapEditor(IFrontend *frontend, IMapManager& mapManager) :
			IUINodeMapEditor(frontend, mapManager) {
	}

	virtual ~UINodeMapEditor() {
	}

	void initNewMap () override {
		IUINodeMapEditor::initNewMap();
		SpriteDefPtr def;
		for (auto i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
			if (SpriteTypes::isLand(i->second->type))
				def = i->second;
		}
		if (!def)
			return;
		for (int w = 0; w < _mapWidth; ++w) {
			for (int h = 0; h < _mapHeight; ++h) {
				placeTileItem(def, nullptr, w, h, getLayer(SpriteTypes::LAND), false, 0.0f);
			}
		}
	}

	virtual IMapContext* getContext(const std::string& mapname) override {
		return new MiniRacerMapContext(mapname);
	}

	virtual const Animation& getEmitterAnimation(const EntityType& type) const {
		return Animations::IDLE;
	}

	virtual void renderPlayer(int x, int y) const override {
	}

	virtual MapEditorLayer getLayer (const SpriteType& type) const {
		if (SpriteTypes::isLand(type))
			return LAYER_BACKGROUND;
		if (SpriteTypes::isRoad(type))
			return LAYER_FOREGROUND;
		if (SpriteTypes::isVehicle(type) || SpriteTypes::isSolid(type) || SpriteTypes::isLights(type))
			return LAYER_DECORATION;
		if (SpriteTypes::isDecal(type))
			return LAYER_DECORATION;
		if (SpriteTypes::isMoveable(type))
			return LAYER_EMITTER;
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
