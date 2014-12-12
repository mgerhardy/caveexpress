#include "engine/client/ui/nodes/UINodeGooglePlayButton.h"
#include "engine/client/ui/windows/listener/GooglePlayListener.h"
#include "engine/common/ConfigManager.h"

UINodeGooglePlayButton::UINodeGooglePlayButton(IFrontend* frontend) :
		UINodeButtonImage(frontend, "icon-google-login") {
	addListener(UINodeListenerPtr(new GooglePlayListener()));
	setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT);
	if (Config.getConfigVar("persister")->getValue() != "googleplay") {
		setVisible(false);
	}
	_googlePlay = Config.getConfigVar("googleplaystate");
}

void UINodeGooglePlayButton::update(uint32_t deltaTime) {
	UINodeButtonImage::update(deltaTime);
	setVisible(!_googlePlay->getBoolValue());
}
