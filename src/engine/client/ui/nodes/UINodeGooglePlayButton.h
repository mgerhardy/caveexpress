#pragma once

#include "engine/client/ui/nodes/UINodeButtonImage.h"
#include "engine/client/ui/windows/listener/GooglePlayListener.h"
#include "engine/common/ConfigVar.h"

class UINodeGooglePlayButton: public UINodeButtonImage {
protected:
	ConfigVarPtr _googlePlay;

public:
	UINodeGooglePlayButton(IFrontend* frontend);

	void update(uint32_t deltaTime) override;
};
