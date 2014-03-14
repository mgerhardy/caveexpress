#pragma once

#include "UINodeLabel.h"
#include "engine/client/ui/BitmapFont.h"

class UINodeKey: public UINodeLabel {
public:
	UINodeKey (IFrontend *frontend, const std::string& keyName, float x, float y, float height);
	virtual ~UINodeKey ();

	// UINode
	float getAutoHeight () const override;
};
