#pragma once

#include "ui/nodes/UINode.h"
#include "common/Pointers.h"
#include "common/Config.h"
#include "common/Payment.h"
#include "common/Logger.h"
#include <vector>
#include <string>
#include <SDL_platform.h>

#define UI_WINDOW_MAP "map"
#define UI_WINDOW_MULTIPLAYER "multiplayer"
#define UI_WINDOW_CREATE_SERVER "createserver"
#define UI_WINDOW_CAMPAIGN "campaign"
#define UI_WINDOW_CAMPAIGN_MAPS "campaignmaps"
#define UI_WINDOW_SETTINGS "settings"
#define UI_WINDOW_OPTIONS "options"
#define UI_WINDOW_PAYMENT "payment"
#define UI_WINDOW_MAIN "main"
#define UI_WINDOW_MODE_SELECTION "modeselection"
#define UI_WINDOW_MAPEDITOR_HELP "mapeditorhelp"
#define UI_WINDOW_MAPEDITOR_OPTIONS "mapeditoroptions"
#define UI_WINDOW_GAMEOVER "gameover"
#define UI_WINDOW_GAMEFINISHED "finished"
#define UI_WINDOW_MAPFINISHED "mapfinished"
#define UI_WINDOW_MAPFAILED "mapfailed"
#define UI_WINDOW_EDITOR "editor"
#define UI_WINDOW_HELP "help"
#define UI_WINDOW_MAP_HELP "maphelp"
#define UI_WINDOW_GOOGLEPLAY "googleplay"

#define WINDOW_FLAG_MODAL			(1 << 0)
#define WINDOW_FLAG_FULLSCREEN		(1 << 1)
#define WINDOW_FLAG_ROOT			(1 << 2)
// defines an important window and popMain stops here
#define WINDOW_FLAG_MAIN			(1 << 3)

typedef uint32_t WindowFlags;

class UIWindow : public UINode {
protected:
	const WindowFlags _flags;
	std::string _onPush;
	std::string _onPop;
	std::string _musicFile;
	int _music;
	uint32_t _pushedTime;
	// millis that no node actions are executed for after the window was pushed onto the stack
	uint32_t _inactiveAfterPush;
	bool _playClickSound;

	UIWindow (const std::string& id, IFrontend *frontend, WindowFlags flags = WINDOW_FLAG_FULLSCREEN);

	bool wantBackButton () const;
	UINode* addTextureNode (const std::string& texture, float x, float y, float w, float h);
	bool isActiveAfterPush () const;

	void setInactiveAfterPush (uint32_t millis = 250)
	{
		_inactiveAfterPush = millis;
	}
public:
	virtual ~UIWindow ();

	virtual void render (int x, int y) const;

	bool isModal () const;
	bool isRoot () const;
	bool isFullscreen () const;
	bool isMain () const;

	void startMusic ();
	void stopMusic ();

	virtual bool shouldDelete () const {
		return false;
	}

	// returning false will prevent the window from getting popped from the stack
	virtual bool onPop () override;
	// returning false will prevent the window from getting pushed onto the stack
	virtual bool onPush () override;
	// called whenever the window is the top window on the stack again - either by push, or by pop
	virtual void onActive ();
	// called whenever another window is pushed on top of this window (only the direct successor)
	virtual void onPushedOver ();

	void showFullscreenAds ();
	void showAds ();
	void hideAds ();

	virtual bool onKeyRelease (int32_t key) override;
	// returns true if a node handled the key press event
	virtual bool onKeyPress (int32_t key, int16_t modifier) override;
	// returns true if a node handled the finger release event
	virtual bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y) override;
	// returns true if a node handled the finger press event
	virtual bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
	virtual bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button) override;
	virtual bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button) override;
	virtual bool onJoystickButtonPress (int x, int y, uint8_t button) override;
	virtual bool onControllerButtonPress (int x, int y, const std::string& button) override;
};

inline bool UIWindow::isActiveAfterPush () const
{
	return _inactiveAfterPush <= 0 || _pushedTime + _inactiveAfterPush <= _time;
}

inline bool UIWindow::isModal () const
{
	return _flags & WINDOW_FLAG_MODAL;
}

inline bool UIWindow::isMain () const
{
	return _flags & WINDOW_FLAG_MAIN;
}

inline bool UIWindow::isRoot () const
{
	return _flags & WINDOW_FLAG_ROOT;
}

inline bool UIWindow::isFullscreen () const
{
	return _flags & WINDOW_FLAG_FULLSCREEN;
}

inline bool UIWindow::wantBackButton () const
{
	return System.wantBackButton();
}
