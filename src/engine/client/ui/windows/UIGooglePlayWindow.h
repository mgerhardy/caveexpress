#pragma once

#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/common/ConfigManager.h"

class UIGooglePlayWindow: public UIWindow {
private:
	ConfigVarPtr _googlePlay;
	UINodeButtonText* _achievements;
#if 0
	UINodeButtonText* _leaderBoards;
#endif
	UINodeButtonText* _disconnect;
public:
	UIGooglePlayWindow (IFrontend *frontend);
	void update (uint32_t deltaTime) override;
};
