#pragma once

#include "ui/windows/UIWindow.h"
#include "ui/nodes/UINodeLabel.h"
#include "common/EntityType.h"
#include "common/Animation.h"
#include "common/Math.h"

/**
 * @brief Help window shown before a map starts. The map stays paused until it is closed.
 *
 * CaveExpress maps define the contents from Lua:
 *
 * @code
 * function intro(help)
 *     help:headline(tr("Objectives"))
 *     help:text(tr("Deliver packages to the shredders"))
 *     if isTouch() then
 *         help:text(tr("Drop them with the second finger"))
 *     else
 *         help:text(tr("Drop them by hitting SPACE bar"))
 *     end
 *     help:beginRow()
 *     help:entity("player", "flying", tr("Player"))
 *     help:entity("item-package", "idle", tr("Package"))
 *     help:endRow()
 *     help:bar(tr("Time bar"), 1, 1, 1, 0.5)
 * end
 * @endcode
 *
 * If @c intro exists in the map script, it is shown automatically. There is no
 * separate C++ window class per tutorial.
 */
class Intro: public UIWindow {
public:
	Intro(const std::string& name, IFrontend* frontend, bool transient = false);

	virtual ~Intro() {
	}

	/** Call from a derived class ctor after the vtable is set up. */
	void init ();

	void onActive () override;
	bool onPop () override;
	bool shouldDelete () const override;

	bool onKeyPress (int32_t key, int16_t modifier) override;
	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override;

	void addHeadline (const std::string& text);
	void addText (const std::string& text);
	void addEntity (const std::string& typeName, const std::string& animationName, const std::string& text);
	void beginRow ();
	void endRow ();
	void addBar (const std::string& text);
	void addBar (const std::string& text, const Color& barColor);

	/** Load @c maps/<name>.lua, run @c intro(help) if present, and push the window. */
	static bool pushFromMapScript (const std::string& mapName, IFrontend* frontend);

protected:
	UINode *_background;
	UINode *_panel;
	UINode *_row;
	bool _transient;

	virtual void addIntroNodes(UINode* /*parent*/) {}
	UINode* contentParent ();
	void addPanel ();
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
