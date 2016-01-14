#pragma once

#include "common/IEventObserver.h"
#include "common/ConfigVar.h"
#include "windows/UIWindow.h"
#include "common/CommandSystem.h"
#include "common/Common.h"
#include "common/Singleton.h"
#include "common/System.h"
#include "common/Log.h"
#include <memory>
#include "common/IProgressCallback.h"
#include "textures/TextureCache.h"
#include "sprites/SpriteCache.h"

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

	virtual void onLater () {}
	virtual void onOk () {}
	virtual void onCancel () {}
};

class UIPopupOkCommandCallback: public UIPopupCallback {
protected:
	const std::string _command;
public:
	explicit UIPopupOkCommandCallback(const std::string& command) :
		_command(command) {
	}

	virtual void onOk () override {
		Commands.executeCommandLine(_command);
		UIPopupCallback::onOk();
	}
};

#define UIPOPUP_OK			(1 << 0)
#define UIPOPUP_CANCEL		(1 << 1)
#define UIPOPUP_LATER		(1 << 2)
#define UIPOPUP_NOCLOSE		(1 << 3)

typedef std::shared_ptr<UIPopupCallback> UIPopupCallbackPtr;

class UI: public IEventObserver, public IProgressCallback, public NonCopyable {
private:
	ServiceProvider *_serviceProvider;
	EventHandler *_eventHandler;
	IFrontend *_frontend;
	TexturePtr _mouseCursor;
	bool _cursor;
	bool _showCursor;
	ConfigVarPtr _mouseSpeed;

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
	bool _motionFinger;

	void printStack ();
	void focusNext (const ICommand::Args& args);
	void focusPrev (const ICommand::Args& args);
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

	mutable SpriteCache _spriteCache;

	bool _restart;
	bool _delayedPop;
	bool _noPushAllowed;

	uint32_t _time;
	uint32_t _lastJoystickMoveTime;
	int _lastJoystickMovementValue;
	bool _rotateFonts;

	typedef std::map<std::string, std::string> LanguageMap;
	LanguageMap _languageMap;

	SDL_threadID _threadId;

	bool _shutdown;

	UI ();
	void pushCmd (const std::string& windowID);
	bool loadGesture (const unsigned char* data, int length);

public:
	virtual ~UI ();
	static UI& get ();

	inline void disableRotatingFonts() { _rotateFonts = false; }

	inline IFrontend *getFrontend () const
	{
		return _frontend;
	}

	template <class UINodeType>
	UINodeType* getNode (const std::string& window, const std::string& nodeId)
	{
		UIWindowMap::iterator i = _windows.find(window);
		if (i == _windows.end()) {
			Log::debug(LOG_UI, "could not find window %s", window.c_str());
			return nullptr;
		}

		UIWindow* windowPtr = i->second;
		UINode* node = windowPtr->getNode(nodeId);
		if (node == nullptr) {
			Log::debug(LOG_UI, "could not find node %s", nodeId.c_str());
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
	void progressInit (int steps, const std::string& text) override;
	void progressStep (const std::string& text) override;
	void progressDone () override;

	// IEventObserver
	void onWindowResize () override;
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
	void onJoystickDeviceRemoved (int32_t device) override;
	void onJoystickButtonPress (uint8_t button) override;
	void onControllerButtonPress (const std::string& button) override;
	/**
	 * @brief pinch/rotate/swipe gestures
	 * @param theta the amount that the fingers rotated during this motion
	 * @param dist the amount that the fingers pinched during this motion
	 * @param numFingers the number of fingers used in the gesture
	 */
	void onMultiGesture (float theta, float dist, int32_t numFingers) override;
	/**
	 * $1 gesture recognition system
	 * @param gestureId a hash of the gesture data. If you have duplicates, just re-record the gesture.
	 * The unique id of the closest gesture to the performed stroke
	 * @param error the difference between the gesture template and the actual performed gesture. Lower error is a better match.
	 * @param numFingers the number of fingers used to draw the stroke.
	 * @sa loadGesture
	 */
	void onGesture (int64_t gestureId, float error, int32_t numFingers) override;
	void onGestureRecord (int64_t gestureId) override;
};

inline TexturePtr UI::loadTexture (const std::string& name) const
{
	return Singleton<TextureCache>::getInstance().load(name);
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

	~UINodePopupListener() {
		_callback = UIPopupCallbackPtr();
	}

	void onClick() override
	{
		switch (_flags) {
		case UIPOPUP_OK:
			_callback->onOk();
			break;
		case UIPOPUP_CANCEL:
			_callback->onCancel();
			break;
		case UIPOPUP_LATER:
			_callback->onLater();
			break;
		default:
			break;
		}
		UI::get().delayedPop();
	}
};
