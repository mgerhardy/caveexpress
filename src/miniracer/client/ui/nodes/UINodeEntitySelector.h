#pragma once

#include "ui/nodes/IUINodeEntitySelector.h"
#include "miniracer/shared/MiniRacerAnimation.h"
#include "miniracer/shared/MiniRacerEntityType.h"

namespace miniracer {

class UINodeEntitySelector: public IUINodeEntitySelector {
protected:
	// can be used to filter out some sprites
	virtual bool shouldBeShown(const EntityType& type) const override {
		return true;
	}

	virtual const Animation& getAnimation(const EntityType& type) const override {
		return Animations::IDLE;
	}

public:
	UINodeEntitySelector(IFrontend *frontend) :
			IUINodeEntitySelector(frontend, -1, -1) {
	}

	virtual ~UINodeEntitySelector() {
	}
};

}
