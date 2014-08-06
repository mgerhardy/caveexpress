#pragma once

#include "engine/common/IEventObserver.h"
#include "engine/common/ConfigVar.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/common/Common.h"
#include "engine/common/System.h"
#include "engine/common/Logger.h"
#include "engine/common/Pointers.h"
#include "engine/common/IProgressCallback.h"
#include "engine/client/textures/TextureCache.h"
#include "engine/client/sprites/SpriteCache.h"

#include <vector>
#include <map>

// forward decl
class EventHandler;
class IFrontend;
class ServiceProvider;
class UINodeBar;

class UIPopupCallback {
public:
	virtual ~UIPopupCallback() {}

	virtual void onOk () {}
	virtual void onCancel () {}
};

#define UIPOPUP_OK			(1 << 0)
#define UIPOPUP_CANCEL		(1 << 1)

typedef SharedPtr<UIPopupCallback> UIPopupCallbackPtr;

class UI: public IEventObserver, public IProgressCallback, public NonCopyable {
private:
	ServiceProvider *_serviceProvider;
	EventHandler *_eventHandler;
	IFrontend *_frontend;
	TexturePtr _mouseCursor;
	bool _cursor;
	bool _showCursor;

	typedef std::map<std::string, UIWindow*> UIWindowMap;
	typedef UIWindowMap::const_iterator UIWindowMapConstIter;
	typedef UIWindowMap::iterator UIWindowMapIter;
	UIWindowMap _windows;

	typedef std::vector<UIWindow*> UIStack;
	typedef UIStack::iterator UIStackIter;
	typedef UIStack::reverse_iterator UIStackReverseIter;
	UIStack _stack;

	int32_t _cursorX;
	int32_t _cursorY;

	void printStack ();
	void focusNext ();
	void focusPrev ();
	void runFocusNode ();

	struct ProgressBar {
		bool active;
		int step;
		int steps;
		std::string text;
	};
	ProgressBar _progress;
	void renderProgress () const;

	typedef std::map<std::string, BitmapFontPtr> Fonts;
	Fonts _fonts;

	mutable TextureCache _textureCache;
	mutable SpriteCache _spriteCache;

	bool _restart;
	bool _delayedPop;
	bool _noPushAllowed;

	uint32_t _time;
	uint32_t _lastJoystickMoveTime;
	int _lastJoystickMovementValue;

	typedef std::map<std::string, std::string> LanguageMap;
	LanguageMap _languageMap;

	UI ();
	void pushCmd (const std::string& windowID);
	bool loadGesture (const std::string& name);

public:
	virtual ~UI ();
	static UI& get ();

	inline IFrontend *getFrontend () const
	{
		return _frontend;
	}

	template <class UINodeType>
	UINodeType* getNode (const std::string& window, const std::string& nodeId)
	{
		UIWindowMap::iterator i = _windows.find(window);
		if (i == _windows.end()) {
			debug(LOG_CLIENT, "could not find window " + window);
			return nullptr;
		}

		UIWindow* windowPtr = i->second;
		UINode* node = windowPtr->getNode(nodeId);
		if (node == nullptr) {
			debug(LOG_CLIENT, "could not find node " + nodeId);
			return nullptr;
		}
		return static_cast<UINodeType*>(node);
	}
	UIWindow* getWindow (const std::string& windowID);
	void addWindow (UIWindow *window);

	void restart ();
	void initRestart ();
	bool initLanguage (const std::string& language);
	void init (ServiceProvider& serviceProvider, EventHandler &eventHandler, IFrontend &frontend);
	void initStack ();
	void shutdown ();
	void update (uint32_t deltaTime);
	void render ();
	TexturePtr loadTexture (const std::string& name) const;
	SpritePtr loadSprite (const std::string& name) const;

	UIWindow* getRootWindow () const;
	bool isMainRoot () const;

	UINodeBar* setBarValue (const std::string& window, const std::string& nodeId, uint16_t value);
	UINodeBar* setBarMax (const std::string& window, const std::string& nodeId, uint16_t max);

	BitmapFontPtr getFont (const std::string& font = "") const;

	// pushes a new root window to the ui stack - and removes all other windows
	// this can e.g. be used to switch between the main windows for e.g. the game and the editor
	void pushRoot (const std::string& windowID);

	// pushes a new window onto the ui stack
	UIWindow* push (const std::string& window);
	// pops the current window and its popups from the ui stack
	void pop ();
	void delayedPop ();

	const std::string translate (const std::string& in) const;

	// pops until important window appears
	void popMain ();

	void showCursor (bool show);
	bool isCursorVisible () const;
	void getCursorPosition (int &x, int &y) const;
	void setCursorPosition (int x, int y);

	void popup (const std::string& text, int flags, UIPopupCallbackPtr callback);

	// IProgressCallback
	void progressInit (int steps, const std::string& text);
	void progressStep (const std::string& text);
	void progressDone ();

	// IEventObserver
	bool onTextInput (const std::string& text) override;
	bool onFingerRelease (int64_t finger, float x, float y) override;
	bool onFingerPress (int64_t finger, float x, float y) override;
	void onFingerMotion (int64_t finger, float x, float y, float dx, float dy) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onKeyRelease (int32_t key) override;
	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;
	void onMouseButtonRelease (int32_t x, int32_t y, uint8_t button) override;
	void onMouseButtonPress (int32_t x, int32_t y, uint8_t button) override;
	void onMouseWheel (int32_t x, int32_t y) override;
	void onJoystickMotion (bool horizontal, int value) override;
	void onJoystickButtonPress (uint8_t button) override;
	void onControllerButtonPress (const std::string& button) override;
	void onGesture (int64_t gestureId) override;
	void onGestureRecord (int64_t gestureId) override;
};

inline TexturePtr UI::loadTexture (const std::string& name) const
{
	return _textureCache.load(name);
}

inline SpritePtr UI::loadSprite (const std::string& name) const
{
	return _spriteCache.load(name);
}

// Use this class in popup buttons as listener to call the popup callback
class UINodePopupListener: public UINodeListener {
private:
	UIPopupCallbackPtr _callback;
	int _flags;
public:
	UINodePopupListener(UIPopupCallbackPtr callback, int flags) :
			_callback(callback), _flags(flags)
	{
	}

	void onClick()
	{
		switch (_flags) {
		case UIPOPUP_OK:
			_callback->onOk();
			break;
		case UIPOPUP_CANCEL:
			_callback->onCancel();
			break;
		default:
			break;
		}
		UI::get().delayedPop();
	}
};
