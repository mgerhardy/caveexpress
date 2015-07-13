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

	virtual IMapContext* getContext(const std::string& mapname) override {
		return new MiniRacerMapContext(mapname);
	}

	virtual const Animation& getEmitterAnimation(const EntityType& type) const {
		return Animations::IDLE;
	}

	virtual void renderPlayer(int x, int y) const override {
	}

	virtual bool isMapTile(const SpriteType &type) const override {
		return SpriteTypes::isMapTile(type);
	}

	virtual bool isPlayer(const EntityType &type) const override {
		return EntityTypes::isPlayer(type);
	}
};

}
