#pragma once

#include "ui/nodes/IUINodeEntitySelector.h"
#include "caveexpress/server/entities/EntityEmitter.h"

namespace caveexpress {

class UINodeEntitySelector: public IUINodeEntitySelector {
private:
	typedef IUINodeEntitySelector Super;
protected:
	bool shouldBeShown(const EntityType& type) const override {
		if (EntityTypes::isPlayer(type))
			return true;
		const EntityType** types = EntityEmitter::getSupportedEntityTypes();
		for (; *types; ++types) {
			if (type == **types)
				return true;
		}

		return false;
	}
	const Animation& getAnimation(const EntityType& type) const override {
		const Animation& animation =
				EntityTypes::hasDirection(type) ?
						Animations::ANIMATION_IDLE_RIGHT :
						Animations::ANIMATION_IDLE;
		return animation;
	}

public:
	UINodeEntitySelector(IFrontend *frontend, int cols = -1, int rows = -1) :
			Super(frontend, cols, rows) {
	}
};

}
