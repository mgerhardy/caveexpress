#pragma once

#include "ui/nodes/IUINodeSpriteSelector.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"

namespace caveexpress {

class UINodeSpriteSelector: public IUINodeSpriteSelector {
private:
	typedef IUINodeSpriteSelector Super;
protected:
	bool shouldBeShown(const SpriteDefPtr& sprite) const override {
		const SpriteType& type = sprite->type;
		if (!SpriteTypes::isMapTile(type) && !SpriteTypes::isLiane(type))
			return false;
		if (SpriteTypes::isGeyser(type) && sprite->isStatic())
			return false;
		if (SpriteTypes::isPackageTarget(type) && !sprite->isStatic())
			return false;

		return true;
	}

public:
	UINodeSpriteSelector(IFrontend *frontend, int cols = -1, int rows = -1) :
			Super(frontend, cols, rows) {
	}
};

}
