#include "client/ui/nodes/UINodeGooglePlayButton.h"
#include "client/ui/windows/listener/GooglePlayListener.h"
#include "common/ConfigManager.h"

UINodeGooglePlayButton::UINodeGooglePlayButton(IFrontend* frontend) :
		UINodeButtonImage(frontend, "icon-google-login") {
	addListener(UINodeListenerPtr(new GooglePlayListener()));
	setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_LEFT);
	if (Config.getConfigVar("persister")->getValue() != "googleplay") {
		setVisible(false);
	}
}
