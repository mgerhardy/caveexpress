#pragma once

#include "ui/nodes/IUINodeEntitySelector.h"
#include "cavepacker/shared/CavePackerAnimation.h"
#include "cavepacker/shared/CavePackerEntityType.h"

namespace cavepacker {

class UINodeEntitySelector: public IUINodeEntitySelector {
protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const EntityType& type) const override {
		return true;
	}

	virtual const Animation& getAnimation(const EntityType& type) const override {
		return Animation::NONE;
	}

public:
	UINodeEntitySelector(IFrontend *frontend) :
			IUINodeEntitySelector(frontend, -1, -1) {
	}

	virtual ~UINodeEntitySelector() {
	}
};

}
