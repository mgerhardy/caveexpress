#pragma once

#include "engine/client/ui/windows/UIWindow.h"

#define UINODE_LIVES "lives"
#define UINODE_PACKAGES "packages"
#define UINODE_SECONDS_REMAINING "seconds"
#define UINODE_MAP "map"
#define UINODE_HITPOINTS "hitpoints"
#define UINODE_COLLECTED "collected"
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
	IMapControl *_mapControl;
	UINodeButtonText *_startButton;
	UINodeLabel *_waitLabel;
	bool _cursorActive;
	ServiceProvider& _serviceProvider;

	virtual UINode* getFingerControl ();
	virtual UINode* getControl ();
	// call this in your derived classes
	void init();
	virtual void initHudNodes();

public:
	IUIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map, IUINodeMap* nodeMap);
	virtual ~IUIMapWindow() {}

	bool isGameActive () const;

	void start ();
	// called for multiplayer -waiting-for-players dialog.
	// the server admin has the option to force a start of the match even if not
	// all players are connected.
	void initWaitingForPlayers (bool adminOptions);

	bool onPop () override;
	void onPushedOver () override;
	void onActive () override;
};
