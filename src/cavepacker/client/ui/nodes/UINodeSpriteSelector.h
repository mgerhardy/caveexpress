#pragma once

#include "ui/nodes/IUINodeSpriteSelector.h"
#include "miniracer/client/commands/CmdMapOpenInEditor.h"

namespace cavepacker {

class UINodeSpriteSelector: public IUINodeSpriteSelector {
protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const SpriteDefPtr& sprite) const override {
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
