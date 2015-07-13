#pragma once

#include "ui/nodes/IUINodeSpriteSelector.h"

namespace miniracer {

class UINodeSpriteSelector: public IUINodeSpriteSelector {
protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const SpriteDefPtr& sprite) const {
		const SpriteType& type = sprite->type;
		return SpriteTypes::isMapTile(type);
	}

public:
	UINodeSpriteSelector (IFrontend *frontend) :
			IUINodeSpriteSelector(frontend, -1, -1) {
	}

	virtual ~UINodeSpriteSelector () {
	}
};

}
