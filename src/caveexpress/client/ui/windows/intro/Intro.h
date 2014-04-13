#pragma once

#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/common/EntityType.h"
#include "engine/common/Animation.h"

class Intro: public UIWindow {
public:
	Intro(const std::string& name, IFrontend* frontend);

	virtual ~Intro() {
	}

	void init ();

protected:
	UINode *_background;
	UINode *_panel;

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
	IntroTypeDescription(IFrontend* frontend, const EntityType& type, const Animation& animation, const std::string& text);
};
