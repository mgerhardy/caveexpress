#pragma once

#include "client/ui/windows/UIWindow.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "common/EntityType.h"
#include "common/Animation.h"

/**
 * @brief This is a window that is shown before the map is starting.
 *
 * As long as it's visible, the map is in a pause mode. Once you close the window,
 * the map is starting.
 *
 * In order to show such a window, you have to tell the map that it should show it
 * on start. There is a special property for that: @c introwindow
 */
class Intro: public UIWindow {
public:
	Intro(const std::string& name, IFrontend* frontend);

	virtual ~Intro() {
	}

	// call this in the ctor of your derived class
	// this ensures, that the vtable of Intro is set up already
	void init ();

	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;
protected:
	UINode *_background;
	UINode *_panel;

	// Add your custom nodes for your intro window implementation
	// Should keep the current style of existing intro windows
	virtual void addIntroNodes(UINode* parent) = 0;
};

class IntroLabel: public UINodeLabel {
public:
	IntroLabel(IFrontend* frontend, const std::string& text) : UINodeLabel(frontend, text) {
		setColor(colorBlack);
		setFont(HUGE_FONT);
	}
};

class IntroLabelHeadline: public UINodeLabel {
public:
	IntroLabelHeadline(IFrontend* frontend, const std::string& text) : UINodeLabel(frontend, text) {
		setColor(colorBlack);
		setFont(LARGE_FONT);
	}
};

class IntroTypeDescription: public UINode {
public:
	IntroTypeDescription(UINode* parent, IFrontend* frontend, const EntityType& type, const Animation& animation, const std::string& text);
};

class IntroBarDescription: public UINode {
public:
	IntroBarDescription(IFrontend* frontend, const Color& barColor, const std::string& text);
	IntroBarDescription(IFrontend* frontend, const std::string& text);
};
