#include "Intro.h"
#include "common/Commands.h"
#include "common/ConfigManager.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/LUALibrary.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeIntroBackground.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/UI.h"
#include "common/SpriteDefinition.h"

namespace {

void addIntroSpriteAndLabel (UINode* row, UINode* parent, IFrontend* frontend, UINodeSprite* sprite, const std::string& text)
{
	const float wp = parent->getWidth() / 5.0f;
	sprite->setAspectRatioSize(wp, wp);
	row->add(sprite);
	UINodeLabel* label = new UINodeLabel(frontend, text, UI::get().getFont(HUGE_FONT));
	label->setColor(colorBlack);
	row->add(label);
}

}

IntroTypeDescription::IntroTypeDescription(UINode* parent, IFrontend* frontend, const EntityType& type, const Animation& animation, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	addIntroSpriteAndLabel(this, parent, frontend, new UINodeSprite(frontend, type, animation), text);
}

IntroTypeDescription::IntroTypeDescription(UINode* parent, IFrontend* frontend, const std::string& spriteName, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	UINodeSprite* sprite = new UINodeSprite(frontend);
	sprite->addSprite(UI::get().loadSprite(spriteName));
	addIntroSpriteAndLabel(this, parent, frontend, sprite, text);
}

IntroBarDescription::IntroBarDescription(IFrontend* frontend, const Color& barColor, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	const float barHeight = 12.0f / (float)_frontend->getHeight();
	const float barWidth = 102.0f / (float)_frontend->getWidth();
	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setMax(100);
	timeBar->setCurrent(100);
	timeBar->setBarColor(barColor);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	timeBar->setSize(barWidth, barHeight);
	add(timeBar);
	UINodeLabel* label = new UINodeLabel(frontend, text, getFont(HUGE_FONT));
	label->setColor(colorBlack);
	add(label);
}

IntroBarDescription::IntroBarDescription(IFrontend* frontend, const std::string& text) :
		UINode(frontend) {
	setLayout(new UIHBoxLayout(0.01f, false, NODE_ALIGN_MIDDLE));
	const float barHeight = 12.0f / (float)_frontend->getHeight();
	const float barWidth = 102.0f / (float)_frontend->getWidth();
	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setMax(100);
	timeBar->setCurrent(100);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	timeBar->setSize(barWidth, barHeight);
	add(timeBar);
	UINodeLabel* label = new UINodeLabel(frontend, text, getFont(HUGE_FONT));
	label->setColor(colorBlack);
	add(label);
}

Intro::Intro(const std::string& name, IFrontend* frontend, bool transient) :
		UIWindow(name, frontend, WINDOW_FLAG_MODAL), _row(nullptr), _transient(transient) {
	_onPop = CMD_START;

	_background = new UINodeIntroBackground(frontend);
	UINode *overlay = new UINode(frontend);
	overlay->setBackgroundColor(colorWhiteAlpha80);
	const float padding = 4.0f / std::max(_frontend->getWidth(), _frontend->getHeight());
	overlay->setSize(_background->getWidth() - 2.0f * padding, _background->getHeight() - 2.0f * padding);
	overlay->setPos(padding, padding);
	_background->add(overlay);
	add(_background);

	UINodeButton* close = new UINodeButton(_frontend);
	close->setImage("icon-close");
	close->setOnActivate(CMD_UI_POP);
	close->alignTo(_background, NODE_ALIGN_RIGHT | NODE_ALIGN_TOP, 0.01f);
	add(close);

	_panel = new UINode(frontend);
	_panel->setStandardPadding();
	_panel->setPos(0.0f, _background->getTop());
	_panel->setSize(_background->getWidth(), _background->getHeight());
	_panel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	UIVBoxLayout *layout = new UIVBoxLayout(0.01f, true, NODE_ALIGN_CENTER);
	_panel->setLayout(layout);
	setInactiveAfterPush(1000L);

	if (wantBackButton()) {
		UINodeMainButton *continueButton = new UINodeMainButton(frontend, tr("Continue"), LARGE_FONT, colorBlack);
		const float gapBack = std::max(0.01f, getScreenPadding());
		continueButton->alignTo(_background, NODE_ALIGN_BOTTOM | NODE_ALIGN_RIGHT, gapBack);
		continueButton->setOnActivate(CMD_UI_POP);
		add(continueButton);
	}
}

bool Intro::shouldDelete () const
{
	return _transient;
}

bool Intro::onPop ()
{
	const bool retVal = UIWindow::onPop();
	Config.setBindingsSpace(BINDINGS_MAP);
	return retVal;
}

void Intro::onActive ()
{
	UIWindow::onActive();
	addLastFocus();
	Config.setBindingsSpace(BINDINGS_UI);
}

bool Intro::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	if (!wantBackButton())
		UI::get().delayedPop();
	return UIWindow::onKeyPress(key, modifier);
}

bool Intro::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	UI::get().delayedPop();
	return UIWindow::onFingerPress(finger, x, y);
}

void Intro::init ()
{
	addIntroNodes(_panel);
	addPanel();
}

void Intro::addPanel ()
{
	endRow();
	add(_panel);
}

UINode* Intro::contentParent ()
{
	return _row != nullptr ? _row : _panel;
}

void Intro::addHeadline (const std::string& text)
{
	contentParent()->add(new IntroLabelHeadline(_frontend, text));
}

void Intro::addText (const std::string& text)
{
	contentParent()->add(new IntroLabel(_frontend, text));
}

void Intro::addEntity (const std::string& typeName, const std::string& animationName, const std::string& text)
{
	const EntityType& type = EntityType::getByName(typeName);
	if (!type.isNone()) {
		const Animation& animation = animationName.empty() ? Animation::NONE : Animation::getByName(animationName);
		if (!animationName.empty() && animation.isNone()) {
			Log::error(LOG_UI, "intro: unknown animation '%s'", animationName.c_str());
			return;
		}
		contentParent()->add(new IntroTypeDescription(_panel, _frontend, type, animation, text));
		return;
	}
	if (SpriteDefinition::get().getSpriteDefinition(typeName)) {
		contentParent()->add(new IntroTypeDescription(_panel, _frontend, typeName, text));
		return;
	}
	Log::error(LOG_UI, "intro: unknown entity type or sprite '%s'", typeName.c_str());
}

void Intro::beginRow ()
{
	endRow();
	_row = new UINode(_frontend);
	_row->setLayout(new UIHBoxLayout(0.01f));
}

void Intro::endRow ()
{
	if (_row == nullptr)
		return;
	_panel->add(_row);
	_row = nullptr;
}

void Intro::addBar (const std::string& text)
{
	contentParent()->add(new IntroBarDescription(_frontend, text));
}

void Intro::addBar (const std::string& text, const Color& barColor)
{
	contentParent()->add(new IntroBarDescription(_frontend, barColor, text));
}

namespace {

int luaTr (lua_State* L)
{
	const char* in = luaL_checkstring(L, 1);
	const std::string out = UI::get().translate(in);
	lua_pushstring(L, out.c_str());
	return 1;
}

Intro* luaIntro (lua_State* L)
{
	return LUA::getUserData<Intro>(L, 1, "Intro");
}

int luaHeadline (lua_State* L)
{
	luaIntro(L)->addHeadline(luaL_checkstring(L, 2));
	return 0;
}

int luaText (lua_State* L)
{
	luaIntro(L)->addText(luaL_checkstring(L, 2));
	return 0;
}

int luaEntity (lua_State* L)
{
	const char* anim = luaL_optstring(L, 3, "idle");
	const char* text = luaL_optstring(L, 4, "");
	luaIntro(L)->addEntity(luaL_checkstring(L, 2), anim, text);
	return 0;
}

int luaBeginRow (lua_State* L)
{
	luaIntro(L)->beginRow();
	return 0;
}

int luaEndRow (lua_State* L)
{
	luaIntro(L)->endRow();
	return 0;
}

int luaBar (lua_State* L)
{
	Intro* intro = luaIntro(L);
	const char* text = luaL_checkstring(L, 2);
	if (lua_gettop(L) >= 6) {
		Color color;
		color.rgba[0] = static_cast<float>(luaL_checknumber(L, 3));
		color.rgba[1] = static_cast<float>(luaL_checknumber(L, 4));
		color.rgba[2] = static_cast<float>(luaL_checknumber(L, 5));
		color.rgba[3] = static_cast<float>(luaL_checknumber(L, 6));
		intro->addBar(text, color);
	} else {
		intro->addBar(text);
	}
	return 0;
}

luaL_Reg introFuncs[] = {
	{ "headline", luaHeadline },
	{ "text", luaText },
	{ "entity", luaEntity },
	{ "beginRow", luaBeginRow },
	{ "endRow", luaEndRow },
	{ "bar", luaBar },
	{ nullptr, nullptr }
};

}

bool Intro::pushFromMapScript (const std::string& mapName, IFrontend* frontend)
{
	if (mapName.empty() || frontend == nullptr)
		return false;

	const std::string mapFile = FS.getMapsDir() + mapName + ".lua";
	if (!FS.exists(mapFile))
		return false;
	LUA lua;
	lua_register(lua.getState(), "tr", luaTr);
	if (!lua.load(mapFile))
		return false;
	if (!lua.hasFunction("intro"))
		return false;

	lua.reg("Intro", introFuncs);

	Intro* window = new Intro("mapintro", frontend, true);
	lua_State* L = lua.getState();
	Intro** udata = LUA::newUserdata<Intro>(L, "Intro");
	*udata = window;
	lua_getglobal(L, "intro");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		delete window;
		return false;
	}
	lua_pushvalue(L, -2);
	lua_remove(L, -3);
	if (lua_pcall(L, 1, 0, 0) != 0) {
		Log::error(LOG_UI, "intro(): %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		delete window;
		return false;
	}

	window->addPanel();
	UI::get().pushTransient(window);
	return true;
}
