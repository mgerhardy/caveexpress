#pragma once

#include "common/ConfigVar.h"
#include "ui/windows/UIWindow.h"

// forward decl
class UINodeButton;

class UIGooglePlayWindow: public UIWindow {
private:
	ConfigVarPtr _googlePlay;
	UINodeButton* _achievements;
#if 0
	UINodeButton* _leaderBoards;
#endif
	UINodeButton* _disconnect;
	UINodeButton* _login;
public:
	explicit UIGooglePlayWindow (IFrontend *frontend);
	void update (uint32_t deltaTime) override;
	bool onPush () override;
};
