#pragma once

#include "client/ui/nodes/UINode.h"
#include "common/Pointers.h"
#include "shared/Payment.h"
#include <vector>
#include <string>
#include <SDL_platform.h>

#define UI_WINDOW_MAP "map"
#define UI_WINDOW_SETTINGS "settings"
#define UI_WINDOW_MAIN "main"
#define UI_WINDOW_ROUNDRESULT "roundresult"
#define UI_WINDOW_VIEWRESULT "viewresult"

#define WINDOW_FLAG_MODAL			(1 << 0)
#define WINDOW_FLAG_FULLSCREEN		(1 << 1)
#define WINDOW_FLAG_ROOT			(1 << 2)
// defines an important window and popMain stops here
#define WINDOW_FLAG_MAIN			(1 << 3)

typedef uint32_t WindowFlags;

class EntityType;
class Animation;
class UINodeSprite;

class UIWindow {
protected:
	typedef std::vector<UINode*> UINodeList;
	typedef UINodeList::iterator UINodeListIter;
	typedef UINodeList::const_iterator UINodeListConstIter;
	typedef UINodeList::reverse_iterator UINodeListRevIter;
	typedef UINodeList::const_reverse_iterator UINodeListConstRevIter;
	UINodeList _nodes;
	const std::string _id;
	const WindowFlags _flags;
	std::string _onPush;
	std::string _onPop;
	std::string _musicFile;
	int _music;
	IFrontend *_frontend;
	float _x;
	float _y;
	float _width;
	float _height;
	uint32_t _time;
	uint32_t _pushedTime;
	// millis that no node actions are executed for after the window was pushed onto the stack
	uint32_t _inactiveAfterPush;

	UIWindow (const std::string& id, IFrontend *frontend, WindowFlags flags = WINDOW_FLAG_FULLSCREEN);

	bool wantBackButton () const;
	void reset ();
	BitmapFontPtr getFont (const std::string& font = "") const;
	UINode* addTextureNode (const std::string& texture, float x, float y, float w, float h);
	bool isActiveAfterPush () const;
public:
	virtual ~UIWindow ();
	UINode* getNode (const std::string& nodeId);
	const std::string& getName () const;

	void addFront (UINode* node);
	void add (UINode* node);
	virtual void render () const;
	virtual void update (uint32_t deltaTime);

	bool isModal () const;
	bool isRoot () const;
	bool isFullscreen () const;
	bool isMain () const;

	void startMusic ();
	void stopMusic ();

	// returning false will prevent the window from getting popped from the stack
	virtual bool onPop ();
	// returning false will prevent the window from getting pushed onto the stack
	virtual bool onPush ();
	// called whenever the window is the top window on the stack again - either by push, or by pop
	virtual void onActive ();

	void showAds ()
	{
		if (getSystem().hasItem(PAYMENT_ADFREE))
			return;

		getSystem().showAds(true, true);
	}

	void hideAds ()
	{
		getSystem().showAds(false, true);
	}

	bool checkFocus (int32_t x, int32_t y);
	void nextFocus ();
	void execute ();

	bool onKeyRelease (int32_t key);
	// returns true if a node handled the key press event
	bool onKeyPress (int32_t key, int16_t modifier);
	// returns true if a node handled the finger release event
	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y);
	// returns true if a node handled the finger press event
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y);
	bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy);
	bool onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY);
	bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button);
	bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button);
	bool onMouseWheel (int32_t x, int32_t y);
	// @param[in] value The amount of pixels that the joystick was moved
	bool onJoystickMotion (int x, int y, bool horizontal, int value);
	bool onJoystickButtonPress (int x, int y, uint8_t button);
	bool onControllerButtonPress (int x, int y, const std::string& button);
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

inline void UIWindow::reset ()
{
	_x = _frontend->getWidth();
	_y = _frontend->getHeight();
	_width = -1;
	_height = -1;
}

inline const std::string& UIWindow::getName () const
{
	return _id;
}

inline bool UIWindow::wantBackButton () const
{
#if defined(__ANDROID__)
	return false;
#else
	return true;
#endif
}
