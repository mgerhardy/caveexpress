#pragma once

#include "engine/client/ui/windows/UIWindow.h"
#include "engine/common/ConfigManager.h"

// forward decl
class UINodeButtonText;
class UINodeButtonImage;

class UIGooglePlayWindow: public UIWindow {
private:
	ConfigVarPtr _googlePlay;
	UINodeButtonText* _achievements;
#if 0
	UINodeButtonText* _leaderBoards;
#endif
	UINodeButtonText* _disconnect;
	UINodeButtonImage *_login;
public:
	UIGooglePlayWindow (IFrontend *frontend);
	void update (uint32_t deltaTime) override;
	bool onPush () override;
};
