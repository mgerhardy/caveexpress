#pragma once

#include "shared/IEventObserver.h"
#include "shared/ConfigVar.h"
#include "client/ui/windows/UIWindow.h"
#include "common/Common.h"
#include "common/System.h"
#include "shared/IProgressCallback.h"
#include "client/textures/TextureCache.h"

#include <vector>
#include <map>

// forward decl
class EventHandler;
class IFrontend;
class ServiceProvider;
class RoundController;

class UI: public IEventObserver, public IProgressCallback, public NonCopyable {
private:
	EventHandler *_eventHandler;
	IFrontend *_frontend;
	TexturePtr _mouseCursor;
	ConfigVarPtr _cursor;
	RoundController *_roundController;

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

	UIWindow* getWindow (const std::string& windowID);
	void addWindow (UIWindow *window);
	void printStack ();
	void focusNext ();
	void execute ();

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

	bool _restart;
	ServiceProvider *_serviceProvider;

	UI ();

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
			System.exit("could not find window " + window, 1);
		}

		UIWindow* windowPtr = i->second;
		UINode* node = windowPtr->getNode(nodeId);
		return static_cast<UINodeType*>(node);
	}

	void restart ();
	void initRestart ();
	void init (ServiceProvider& serviceProvider, EventHandler &eventHandler, IFrontend &frontend);
	void initStack ();
	void shutdown ();
	void update (uint32_t deltaTime);
	void render ();
	TexturePtr loadTexture (const std::string& name) const;

	UIWindow* getRootWindow () const;
	bool isMainRoot () const;

	void setBarValue (const std::string& window, const std::string& nodeId, uint16_t value);
	void setBarMax (const std::string& window, const std::string& nodeId, uint16_t max);

	BitmapFontPtr getFont (const std::string& font = "") const;

	// pushes a new root window to the ui stack - and removes all other windows
	// this can e.g. be used to switch between the main windows for e.g. the game and the editor
	void pushRoot (const std::string& windowID);

	// pushes a new window onto the ui stack
	void push (const std::string& window);
	// pops the current window and its popups from the ui stack
	void pop ();

	// pops until important window appears
	void popMain ();

	void showCursor (bool show);
	void setCursorPosition (int x, int y);

	// IProgressCallback
	void progressInit (int steps, const std::string& text);
	void progressStep (const std::string& text);
	void progressDone ();

	// IEventObserver
	bool onFingerRelease (int64_t finger, float x, float y) override;
	bool onFingerPress (int64_t finger, float x, float y) override;
	void onFingerMotion (int64_t finger, float x, float y, float dx, float dy) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onKeyRelease (int32_t key) override;
	void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY) override;
	void onMouseButtonRelease (int32_t x, int32_t y, uint8_t button) override;
	void onMouseButtonPress (int32_t x, int32_t y, uint8_t button) override;
	void onMouseWheel (int32_t x, int32_t y) override;
	void onJoystickMotion (bool horizontal, float value) override;
	void onJoystickButtonPress (uint8_t button) override;
	void onControllerButtonPress (const std::string& button) override;
};

inline TexturePtr UI::loadTexture (const std::string& name) const
{
	return _textureCache.load(name);
}
