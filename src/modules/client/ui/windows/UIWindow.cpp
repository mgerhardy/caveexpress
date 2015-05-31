#include "UIWindow.h"
#include "client/ui/UI.h"
#include "client/sound/Sound.h"
#include "common/Logger.h"
#include "common/EventHandler.h"
#include "common/CommandSystem.h"
#include "common/System.h"
#include "common/ConfigManager.h"
#include "client/ui/BitmapFont.h"

UIWindow::UIWindow (const std::string& id, IFrontend *frontend, WindowFlags flags) :
		UINode(frontend, id), _flags(flags), _musicFile("music-1"), _music(-1), _pushedTime(0), _inactiveAfterPush(0), _playClickSound(true)
{
}

UIWindow::~UIWindow ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete *i;
	}
	_nodes.clear();
}

void UIWindow::render (int x, int y) const
{
	if (_flags & WINDOW_FLAG_MODAL) {
		const Color bgColor = {0.7f, 0.7f, 0.7f, 0.4f};
		_frontend->renderFilledRect(0, 0, 0, 0, bgColor);
	}

	UINode::render(x, y);

	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		nodePtr->renderOnTop(x, y);
	}
}

bool UIWindow::onPop ()
{
	bool pop = true;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const bool val = (*i)->onPop();
		if (!val)
			pop = false;
	}
	if (pop) {
		Commands.executeCommandLine(_onPop);
		_pushedTime = 0;
	}

	return pop;
}

void UIWindow::onPushedOver ()
{
}

void UIWindow::onActive ()
{
	System.track("activewindow", getId());
	_pushedTime = _time;
}

bool UIWindow::onPush ()
{
	bool push = true;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const bool val = (*i)->onPush();
		if (!val)
			push = false;
	}
	if (push) {
		Commands.executeCommandLine(_onPush);
		startMusic();
		addFirstFocus();
	}

	return push;
}

void UIWindow::startMusic ()
{
	if (!_musicFile.empty())
		_music = SoundControl.playMusic(_musicFile);
	else
		stopMusic();
}

void UIWindow::stopMusic ()
{
	SoundControl.haltMusic(_music);
	_music = -1;
}

bool UIWindow::onKeyRelease (int32_t key)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onKeyRelease(key);
}

bool UIWindow::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onKeyPress(key, modifier);
}

bool UIWindow::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	if (UINode::onFingerRelease(finger, x, y)) {
		if (_playClickSound)
			SoundControl.play("click");
		return true;
	}

	return false;
}

bool UIWindow::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onFingerPress(finger, x, y);
}

bool UIWindow::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!isActiveAfterPush())
		return false;

	if (UINode::onMouseButtonRelease(x, y, button)) {
		if (_playClickSound)
			SoundControl.play("click");
		return true;
	}

	return false;
}

bool UIWindow::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onMouseButtonPress(x, y, button);
}

bool UIWindow::onJoystickButtonPress (int x, int y, uint8_t button)
{
	if (!isActiveAfterPush())
		return false;

	if (UINode::onJoystickButtonPress(x, y, button)) {
		if (_playClickSound)
			SoundControl.play("click");
		return true;
	}

	return false;
}

bool UIWindow::onControllerButtonPress (int x, int y, const std::string& button)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onControllerButtonPress(x, y, button);
}

UINode* UIWindow::addTextureNode (const std::string& texture, float x, float y, float w, float h)
{
	UINode* imageNode = new UINode(_frontend);
	imageNode->setImage(texture);
	imageNode->setPos(x, y);
	imageNode->setSize(w, h);
	add(imageNode);
	return imageNode;
}

void UIWindow::showFullscreenAds ()
{
	if (getSystem().hasItem(PAYMENT_ADFREE)) {
		debug(LOG_CLIENT, "skip ads");
		return;
	}

	if (!getSystem().showFullscreenAds())
		error(LOG_CLIENT, "failed to show the fullscreen ads");
	else
		info(LOG_CLIENT, "show fullscreen ads");
}

void UIWindow::showAds ()
{
	if (getSystem().hasItem(PAYMENT_ADFREE)) {
		debug(LOG_CLIENT, "skip ads");
		return;
	}
	getSystem().showAds(true);
	info(LOG_CLIENT, "show ads");
}

void UIWindow::hideAds ()
{
	getSystem().showAds(false);
	info(LOG_CLIENT, "hide ads");
}
