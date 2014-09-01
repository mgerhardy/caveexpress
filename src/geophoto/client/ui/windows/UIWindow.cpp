#include "UIWindow.h"
#include "client/ui/UI.h"
#include "client/sound/Sound.h"
#include "shared/Logger.h"
#include "shared/EventHandler.h"
#include "shared/CommandSystem.h"
#include "common/System.h"
#include "shared/Animation.h"

UIWindow::UIWindow (const std::string& id, IFrontend *frontend, WindowFlags flags) :
		_id(id), _flags(flags), _musicFile("music-1"), _music(-1), _frontend(frontend), _time(0), _pushedTime(0), _inactiveAfterPush(0)
{
	reset();
}

UIWindow::~UIWindow ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete *i;
	}
	_nodes.clear();
}

void UIWindow::addFront (UINode* node)
{
	_nodes.insert(_nodes.begin(), node);
}

void UIWindow::add (UINode* node)
{
	_nodes.push_back(node);
}

void UIWindow::render () const
{
	if (_flags & WINDOW_FLAG_MODAL) {
		const Color bgColor = {0.7f, 0.7f, 0.7f, 0.4f};
		_frontend->renderFilledRect(0, 0, 0, 0, bgColor);
	}

	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		nodePtr->render(0, 0);
	}
}

UINode* UIWindow::getNode (const std::string& nodeId)
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = (*i)->getNode(nodeId);
		if (node)
			return node;
	}

	System.exit("could not find node " + nodeId, 1);

	return static_cast<UINode*>(0);
}

void UIWindow::update (uint32_t deltaTime)
{
	_time += deltaTime;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		// even if they are invisible, they get the update call (to decide
		// whether they e.g. wanna get visible again
		(*i)->update(deltaTime);
	}

	reset();
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		_x = std::min(_x, nodePtr->getX());
		_y = std::min(_y, nodePtr->getY());
		const float w = nodePtr->getWidth();
		const float h = nodePtr->getHeight();
		_width = std::max(_width, w <= 0.0f ? _frontend->getWidth() : w);
		_height = std::max(_height, h <= 0.0f ? _frontend->getHeight() : h);
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

void UIWindow::onActive ()
{
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

	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		if (nodePtr->onKeyRelease(key))
			return true;
	}

	return false;
}

bool UIWindow::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		if (nodePtr->onKeyPress(key, modifier))
			return true;
	}

	return false;
}

bool UIWindow::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		if (!nodePtr->checkBounds(x, y))
			continue;
		if (nodePtr->onFingerRelease(finger, x, y)) {
			SoundControl.play("click");
			return true;
		}
	}

	return false;
}

bool UIWindow::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible() || !nodePtr->wantFocus())
			continue;
		if (!nodePtr->checkBounds(x, y))
			continue;
		if (nodePtr->onFingerPress(finger, x, y))
			return true;
	}

	return false;
}

bool UIWindow::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	const bool focus = checkFocus(x, y);
	if (!focus)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->onFingerMotion(finger, x, y, dx, dy);
	}
	return focus;
}

void UIWindow::execute ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->execute();
	}
}

void UIWindow::nextFocus ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;

		if (!nodePtr->hasMultipleFocus()) {
			nodePtr->removeFocus();
			for (;;) {
				++i;
				if (i == _nodes.end())
					i = _nodes.begin();
				nodePtr = *i;
				if (!nodePtr->wantFocus())
					continue;
				break;
			}
		}
		nodePtr->addFocus();
		return;
	}

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->wantFocus())
			continue;
		nodePtr->addFocus();
		break;
	}
}

bool UIWindow::checkFocus (int32_t x, int32_t y)
{
	bool focusAdded = false;
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible()) {
			const bool hasFocus = nodePtr->hasFocus();
			if (hasFocus)
				nodePtr->removeFocus();
		} else if (!focusAdded && nodePtr->wantFocus() && nodePtr->checkBounds(x, y)) {
			focusAdded = true;
			nodePtr->addFocus(x, y);
		} else if (nodePtr->hasFocus()) {
			nodePtr->removeFocus();
		}
	}
	return focusAdded;
}

bool UIWindow::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	const bool focus = checkFocus(x, y);
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->onMouseMotion(x, y, relX, relY);
	}
	return focus;
}

bool UIWindow::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if (nodePtr->onMouseButtonRelease(x, y, button)) {
			SoundControl.play("click");
			return true;
		}
	}

	return false;
}

bool UIWindow::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onMouseButtonPress(x, y, button)) {
			return true;
		}
	}

	return false;
}

bool UIWindow::onMouseWheel (int32_t x, int32_t y)
{
	bool handled = false;
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		handled |= (*i)->onMouseWheel(x, y);
	}
	return handled;
}

bool UIWindow::onJoystickMotion (int x, int y, bool horizontal, int value)
{
	const bool focus = checkFocus(x, y);
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->onJoystickMotion(horizontal, value);
	}
	return focus;
}

bool UIWindow::onJoystickButtonPress (int x, int y, uint8_t button)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onJoystickButtonPress(x, y, button)) {
			SoundControl.play("click");
			return true;
		}
	}
	return false;
}

bool UIWindow::onControllerButtonPress (int x, int y, const std::string& button)
{
	if (!isActiveAfterPush())
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onControllerButtonPress(x, y, button)) {
			return true;
		}
	}
	return false;
}

BitmapFontPtr UIWindow::getFont (const std::string& font) const
{
	return UI::get().getFont(font);
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
