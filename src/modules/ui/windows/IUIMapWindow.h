#pragma once

#include "ui/windows/UIWindow.h"

#define UINODE_LIVES "lives"
#define UINODE_PACKAGES "packages"
#define UINODE_SECONDS_REMAINING "seconds"
#define UINODE_MAP "map"
#define UINODE_HITPOINTS "hitpoints"
#define UINODE_COLLECTED "collected"
#define UINODE_TARGETCAVEID "targetcave"
#define UINODE_POINTS "points"

// forward decl
class UINode;
class UINodeLabel;
class IUINodeMap;
class CampaignManager;
class ServiceProvider;
class UINodeButtonText;
class ClientMap;
class IMapControl;

class IUIMapWindow: public UIWindow {
protected:
	friend class UIMapHelpWindow;
	IUINodeMap *_nodeMap;
	UINodeLabel *_waitLabel;
	IMapControl *_mapControl;
	UINodeButtonText *_startButton;
	bool _cursorActive;
	ServiceProvider& _serviceProvider;
	UINode* _panel;
	uint32_t _lastFingerPressEvent;

	virtual UINode* getFingerControl ();
	virtual UINode* getControl ();
	// call this in your derived classes
	void init();
	virtual void initHudNodes();
	// override this if you need nodes on top of the IMapControl nodes
	virtual void initInputHudNodes();
	virtual void showCursor (bool show);
public:
	IUIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map, IUINodeMap* nodeMap);
	virtual ~IUIMapWindow() {}

	virtual void hideHud();
	virtual void showHud();

	virtual void start ();
	// called for multiplayer -waiting-for-players dialog.
	// the server admin has the option to force a start of the match even if not
	// all players are connected.
	virtual void initWaitingForPlayers (bool adminOptions);

	virtual bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	virtual bool onMultiGesture (float theta, float dist, int32_t numFingers) override;
	virtual bool onPop () override;
	virtual void onPushedOver () override;
	virtual void onActive () override;
	virtual bool onMouseWheel (int32_t x, int32_t y) override;
};
