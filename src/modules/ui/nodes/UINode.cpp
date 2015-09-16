#include "UINode.h"
#include "ui/UI.h"
#include "common/CommandSystem.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/IFrontend.h"
#include "common/Payment.h"
#include "ui/BitmapFont.h"
#include "ui/layouts/IUILayout.h"
#include <SDL.h>
#include <algorithm>

int UINode::_counter = 0;

UINode::UINode (IFrontend *frontend, const std::string& id) :
		_padding(0.0f), _marginTop(0.0f), _marginLeft(0.0f), _onActivate(""), _focus(false), _focusAlpha(0.0f), _focusMouseX(-1), _focusMouseY(-1), _visible(
				true), _enabled(true), _dragStartX(-1), _dragStartY(-1), _alpha(1.0f), _previousAlpha(1.0f), _id(id), _renderBorder(
				false), _frontend(frontend), _align(NODE_ALIGN_LEFT), _time(0), _flashMillis(0), _originalAlpha(-1.0f), _autoId(false), _layout(nullptr), _parent(nullptr),
				_fingerPressed(false), _mousePressed(false)
{
	setPos(0.0f, 0.0f);
	setSize(0.0f, 0.0f);
	Vector4Set(colorWhite, _borderColor);
	Vector4Set(colorNull, _backgroundColor);

	if (_id.empty()) {
		_id = string::toString(_counter++);
		_autoId = true;
	}
}

UINode::~UINode ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete *i;
	}
	_nodes.clear();
	delete _layout;
	_texture = TexturePtr();
}

TexturePtr UINode::loadTexture (const std::string& name) const
{
	return UI::get().loadTexture(name);
}

BitmapFontPtr UINode::getFont (const std::string& font) const
{
	return UI::get().getFont(font);
}

bool UINode::isSmallScreen() const
{
	return System.isSmallScreen(_frontend);
}

void UINode::setPos (float x, float y)
{
	_pos.x = clamp(x, 0.0f, 1.0f);
	_pos.y = clamp(y, 0.0f, 1.0f);
	updateAlignment();
}

void UINode::setSize (float w, float h)
{
	_size.x = clamp(w, 0.0f, 1.0f);
	_size.y = clamp(h, 0.0f, 1.0f);
	updateAlignment();
}

void UINode::setAlignment (int align)
{
	_align = align;
	updateAlignment();
}

bool UINode::hasImage () const
{
	return _texture && _texture->isValid();
}

const TexturePtr& UINode::setImage (const std::string& texture)
{
	if (_autoId) {
		_id = texture;
	}
	_texture = loadTexture(texture);
	if (!hasImage())
		return _texture;

	float w = getWidth();
	if (w <= 0.0f) {
		w = getAutoWidth();
	}
	float h = getHeight();
	if (h <= 0.0f) {
		h = getAutoHeight();
	}
	setSize(w, h);
	updateAlignment();

	return _texture;
}

void UINode::update (uint32_t deltaTime)
{
	_time += deltaTime;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		// even if they are invisible, they get the update call (to decide
		// whether they e.g. wanna get visible again
		(*i)->update(deltaTime);
	}

	for (DelayedTextsIter i = _texts.begin(); i != _texts.end();) {
		i->delayMillis -= std::min(i->delayMillis, deltaTime);
		if (i->delayMillis == 0)
			i = _texts.erase(i);
		else
			++i;
	}

	if (_flashMillis > 0) {
		_flashMillis -= std::min(_flashMillis, deltaTime);
		const float hz = 1.0f;
		const float phase = _time * 0.001f * hz;
		const float moduloPhase = phase - floor(phase);
		_alpha = (1.0 - cos(moduloPhase * (2 * M_PI))) / 2.0;
	} else if (_originalAlpha >= 0.0f) {
		_flashMillis = 0;
		_alpha = _originalAlpha;
		_originalAlpha = -1.0f;
	}
}

float UINode::getParentWidthf () const
{
	if (_parent == nullptr)
		return 1.0f;

	return _parent->getRenderWidth();
}

float UINode::getParentHeightf () const
{
	if (_parent == nullptr)
		return 1.0f;

	return _parent->getRenderHeightf();
}

float UINode::getParentXf () const
{
	if (_parent == nullptr)
		return 0.0f;

	return _parent->getRenderXf();
}

float UINode::getParentYf () const
{
	if (_parent == nullptr)
		return 0.0f;

	return _parent->getRenderYf();
}

void UINode::updateAlignment ()
{
	float x;
	float y;
	if (_align & NODE_ALIGN_CENTER) {
		x = 0.5f - getRenderWidthf(false) / 2.0f;
	} else if (_align & NODE_ALIGN_RIGHT) {
		x = 1.0f - getRenderWidthf(false);
	} else {
		// default is left
		x = getRenderXf(false);
	}

	if (_align & NODE_ALIGN_MIDDLE) {
		y = 0.5f - getRenderHeightf(false) / 2.0f;
	} else if (_align & NODE_ALIGN_BOTTOM) {
		y = 1.0f - getRenderHeightf(false);
	} else {
		// default is top
		y = getRenderYf(false);
	}

	_pos.x = x - _marginLeft;
	_pos.y = y - _marginTop;
}

void UINode::doLayout ()
{
	if (_layout)
		_layout->layout(this);
}

void UINode::onAdd()
{
	doLayout();
}

void UINode::displayText (const std::string& text, uint32_t delayMillis, float x, float y)
{
	struct isEqual {
		explicit isEqual(const std::string& s) :
				_s(s) {
		}

		bool operator()(const UINodeDelayedText& l) {
			return l.text == _s;
		}

		const std::string& _s;
	};
	DelayedTextsIter i = std::find_if(_texts.begin(), _texts.end(), isEqual(text));
	if (i != _texts.end()) {
		i->delayMillis = delayMillis;
		return;
	}
	Log::info(LOG_UI, "Display text '%s' for %u ms", text.c_str(), delayMillis);
	const NodeCoord c(x, y);
	const BitmapFontPtr& font = getFont(HUGE_FONT);
	_texts.push_back(UINodeDelayedText(text, delayMillis, c, font));
}

void UINode::flash (uint32_t flashMillis)
{
	_originalAlpha = _alpha;
	_flashMillis = flashMillis;
}

void UINode::alignTo (const UINode* node, int align, float padding)
{
	float x;
	float y;
	if (align & NODE_ALIGN_CENTER) {
		x = node->getRenderCenterXf() - getRenderWidthf() / 2.0f;
	} else if (align & NODE_ALIGN_RIGHT) {
		x = node->getRenderXf() + node->getRenderWidthf() - getRenderWidthf(false) - padding;
	} else {
		// default is left
		x = node->getRenderXf() + padding;
	}

	if (align & NODE_ALIGN_MIDDLE) {
		y = node->getRenderCenterYf() - getRenderHeightf() / 2.0f;
	} else if (align & NODE_ALIGN_BOTTOM) {
		y = node->getRenderYf() + node->getRenderHeightf() - getRenderHeightf(false) - padding;
	} else {
		// default is top
		y = node->getRenderYf() + padding;
	}

	setPos(x, y);
}

void UINode::renderOnTop (int x, int y) const
{
	const int relX = x + getRenderX();
	const int relY = y + getRenderY();
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		nodePtr->renderOnTop(relX, relY);
	}

	const BitmapFontPtr& font = getFont(MEDIUM_FONT);
	if (_tooltip.empty())
		return;
	if (!hasFocus())
		return;
	const int padding = 2;
	const int width = font->getTextWidth(_tooltip) + 2 * padding;
	const int height = font->getTextHeight(_tooltip) + 2 * padding;
	const int xTooltip = std::min(_frontend->getWidth(), x + _focusMouseX + width) - width;
	const int yTooltip = std::max(0, y + _focusMouseY - height);
	renderFilledRect(xTooltip - padding, yTooltip - padding, width, height, colorBlack);
	font->print(_tooltip, colorWhite, xTooltip, yTooltip);
}

void UINode::renderBack (int x, int y) const
{
	const int w = getRenderWidth(false);
	const int h = getRenderHeight(false);
	if (_backgroundColor[3] > 0.001f) {
		renderFilledRect(x + getRenderX(false), y + getRenderY(false), w, h, _backgroundColor);
	}
}

void UINode::renderMiddle (int x, int y) const
{
	if (!_texture)
		return;
	const int childX = x + getRenderX();
	const int childY = y + getRenderY();
	renderImage(_texture, childX, childY, getRenderWidth(), getRenderHeight(), _alpha);
}

void UINode::renderTop (int x, int y) const
{
	const int childX = x + getRenderX();
	const int childY = y + getRenderY();

	if (_renderBorder) {
		const int w = getRenderWidth(false);
		const int h = getRenderHeight(false);
		renderRect(x + getRenderX(false), y + getRenderY(false), w, h, _borderColor);
	}

	int textYOffset = 0;
	for (DelayedTextsConstIter i = _texts.begin(); i != _texts.end(); ++i) {
		const UINodeDelayedText& t = *i;
		const int fontX = t.pos.x > 0.00001f ? (t.pos.x * _frontend->getWidth()) : (getRenderCenterX() - t.font->getTextWidth(t.text) / 2.0f);
		int fontY;
		if (t.pos.y > 0.00001f) {
			fontY = t.pos.y * _frontend->getHeight();
		} else {
			const int textHeight = t.font->getTextHeight(t.text);
			fontY = textYOffset + getRenderCenterY() - textHeight / 2.0f;
			textYOffset += textHeight;
		}
		t.font->print(t.text, colorWhite, fontX, fontY);
	}

	const bool debug = Config.isDebugUI();
	const bool focus = hasFocus();
	if (debug && focus)
		renderDebug(x, y, y + 20);

	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		nodePtr->render(childX, childY);
	}
}

void UINode::render (int x, int y) const
{
	if (!isVisible())
		return;
	renderBack(x, y);
	renderMiddle(x, y);
	renderTop(x, y);
}

void UINode::renderDebug (int x, int y, int textY) const
{
	const int panelX = getRenderX();
	const int panelY = getRenderY();
	const int w = getRenderWidth(false);
	const int h = getRenderHeight(false);

	const Color* color[5];
	color[0] = &colorGreen;
	color[1] = &colorBlue;
	color[2] = &colorRed;
	color[3] = &colorYellow;
	color[4] = &colorCyan;

	const int index = (panelX * 22 * h + panelY * 23 * w) % 5;
	renderRect(x + getRenderX(false), y + getRenderY(false), w, h, *color[index]);
	if (!fequals(_padding, 0.0f)) {
		renderRect(x + panelX, y + panelY, getRenderWidth(), getRenderHeight(), *color[index]);
	}
	renderFilledRect(x + getRenderCenterX(), y + getRenderCenterY(), 4, 4, colorRed);
	renderFilledRect(x + panelX, y + panelY, 4, 4, colorBlue);

	const BitmapFontPtr& font = getFont(MEDIUM_FONT);
	int panelTextY = textY;
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		const UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->renderDebug(x + panelX, y + panelY, panelTextY);
		const std::string debugInfo = "[id=" + nodePtr->getId() + "]";
		_frontend->renderFilledRect(x - 1, panelTextY + 1, font->getTextWidth(debugInfo) + 2, font->getTextHeight(debugInfo) + 2, colorGrayAlpha);
		font->print(debugInfo, colorCyan, x, panelTextY);
		panelTextY += font->getTextHeight(debugInfo);
	}
}

void UINode::renderRect (int x, int y, int w, int h, const Color& color) const
{
	const float alpha = fequals(_alpha, 1.0f) ? color[3] : _alpha;
	const Color alphaColor = { color[0], color[1], color[2], alpha };
	_frontend->renderRect(x, y, w, h, alphaColor);
}

void UINode::renderFilledRect (int x, int y, int w, int h, const Color& color) const
{
	const float alpha = fequals(_alpha, 1.0f) ? color[3] : _alpha;
	const Color alphaColor = { color[0], color[1], color[2], alpha };
	_frontend->renderFilledRect(x, y, w, h, alphaColor);
}

void UINode::renderLine (int x1, int y1, int x2, int y2, const Color& color) const
{
	const float alpha = fequals(_alpha, 1.0f) ? color[3] : _alpha;
	const Color alphaColor = { color[0], color[1], color[2], alpha };
	_frontend->renderLine(x1, y1, x2, y2, alphaColor);
}

void UINode::renderImage (const TexturePtr& texture, int x, int y, int w, int h, float alpha) const
{
	if (!texture || !texture->isValid())
		return;

	if (w == -1)
		w = texture->getWidth();
	if (h == -1)
		h = texture->getHeight();

	_frontend->renderImage(texture.get(), x, y, w, h, 0, alpha);
}

void UINode::renderImage (const std::string& texture, int x, int y, int w, int h, float alpha) const
{
	const TexturePtr& t = loadTexture(texture);
	renderImage(t, x, y, w, h, alpha);
}

void UINode::initDrag (int32_t x, int32_t y)
{
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (!node->hasFocus())
			continue;
		node->initDrag(x - getRenderX(), y - getRenderY());
		break;
	}

	_dragStartX = x;
	_dragStartY = y;
}

void UINode::handleDrop (uint16_t x, uint16_t y)
{
	_dragStartX = -1;
	_dragStartY = -1;
}

bool UINode::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	_fingerPressed = true;
	initDrag(x, y);
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->isVisible())
			continue;
		if (!nodePtr->checkBounds(x - getRenderX(), y - getRenderY()))
			continue;
		if (nodePtr->onFingerPress(finger, x - getRenderX(), y - getRenderY()))
			return true;
	}

	return false;
}

bool UINode::prevFocus (bool cursorup)
{
	if (_nodes.empty()) {
		if (!isActive())
			return false;
		if (hasFocus())
			return false;
		addFocus(0, 0);
		return true;
	}

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		// search the node that currently has the focus
		if (!nodePtr->hasFocus())
			continue;

		if (nodePtr->prevFocus(cursorup)) {
			addFocus(0, 0);
			return true;
		}

		nodePtr->removeFocus();
		for (++i; i != _nodes.rend(); ++i) {
			UINode* focusNodePtr = *i;
			if (focusNodePtr->addLastFocus()) {
				addFocus(0, 0);
				return true;
			}
		}
		break;
	}

	removeFocus();

	return false;
}

bool UINode::nextFocus (bool cursordown)
{
	if (_nodes.empty()) {
		if (!isActive())
			return false;
		if (hasFocus())
			return false;
		addFocus(0, 0);
		return true;
	}

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		// search the node that currently has the focus
		if (!nodePtr->hasFocus())
			continue;

		if (nodePtr->nextFocus(cursordown)) {
			addFocus(0, 0);
			return true;
		}

		nodePtr->removeFocus();
		for (++i; i != _nodes.end(); ++i) {
			UINode* focusNodePtr = *i;
			if (focusNodePtr->addFirstFocus()) {
				addFocus(0, 0);
				return true;
			}
		}
		break;
	}

	removeFocus();

	return false;
}

bool UINode::addLastFocus ()
{
	bool focus = false;
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		nodePtr->removeFocus();
		if (!focus) {
			focus = nodePtr->addLastFocus();
			if (focus) {
				addFocus(0, 0);
			}
		}
	}

	if (_nodes.empty() && isActive()) {
		addFocus(0, 0);
		return true;
	}
	return focus;
}

bool UINode::addFirstFocus ()
{
	bool focus = false;
	for (UINode* nodePtr : _nodes) {
		nodePtr->removeFocus();
		if (focus)
			continue;
		focus = nodePtr->addFirstFocus();
		if (focus)
			addFocus(0, 0);
	}

	if (_nodes.empty() && isActive()) {
		addFocus(0, 0);
		return true;
	}
	return focus;
}

bool UINode::isActive () const
{
	if (!_enabled)
		return false;

	if (!isVisible())
		return false;

	for (const UINode* nodePtr : _nodes) {
		if (nodePtr->isActive()) {
			return true;
		}
	}

	if (!_onActivate.empty())
		return true;

	if (!_listeners.empty())
		return true;

	return false;
}

bool UINode::runFocusNode ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* nodePtr = *i;
		if (nodePtr->runFocusNode())
			return true;
	}
	if (!hasFocus())
		return false;
	return execute();
}

bool UINode::checkFocus (int32_t x, int32_t y)
{
	if (x <= -1 || y <= -1 || !isVisible() || !isActive()) {
		if (hasFocus()) {
			if (x <= -1 || y <= -1)
				Log::debug(LOG_UI, "remove focus from %s due to invalid coords", getId().c_str());
			else if (!isVisible())
				Log::debug(LOG_UI, "remove focus from invisible node %s", getId().c_str());
			else if (!isActive())
				Log::debug(LOG_UI, "remove focus from inactive node %s", getId().c_str());
			removeFocus();
		}
		return false;
	}

	if (checkBounds(x, y)) {
		if (_nodes.empty()) {
			addFocus(x, y);
			return true;
		}

		const int childX = x - getRenderX();
		const int childY = y - getRenderY();
		bool focusOnChildren = false;
		for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
			UINode* nodePtr = *i;
			int focusX = childX;
			int focusY = childY;
			if (focusOnChildren) {
				focusX = focusY = -1;
			}
			const bool focus = nodePtr->checkFocus(focusX, focusY);
			focusOnChildren |= focus;
		}

		if (focusOnChildren) {
			if (isActive()) {
				addFocus(x, y);
				return true;
			}
		}
	}

	if (hasFocus()) {
		Log::debug(LOG_UI, "nothing left that wants the focus - so remove it from node %s", getId().c_str());
		removeFocus();
	}
	return false;
}

bool UINode::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	const bool focus = checkFocus(x, y);
	if (!focus)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->onFingerMotion(finger, x - getRenderX(), y - getRenderY(), dx, dy);
	}
	return focus;
}

bool UINode::onTextInput (const std::string& text)
{
	for (UINode* nodePtr : _nodes) {
		if (!nodePtr->hasFocus())
			continue;
		if (nodePtr->onTextInput(text))
			return true;
	}

	return false;
}

bool UINode::onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion)
{
	_fingerPressed = false;
	handleDrop(x, y);
	const bool retVal = execute();
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if (nodePtr->onFingerRelease(finger, x - getRenderX(), y - getRenderY(), motion)) {
			return true;
		}
	}

	return retVal;
}

bool UINode::onKeyPress (int32_t key, int16_t modifier)
{
	for (UINode* nodePtr : _nodes) {
		if (!nodePtr->isVisible())
			continue;
		if (nodePtr->onKeyPress(key, modifier))
			return true;
	}

	return false;
}

bool UINode::onKeyRelease (int32_t key)
{
	for (UINode* nodePtr : _nodes) {
		if (!nodePtr->isVisible())
			continue;
		if (nodePtr->onKeyRelease(key))
			return true;
	}

	return false;
}

bool UINode::onPop ()
{
	_texts.clear();
	bool pop = true;
	for (UINode* node : _nodes) {
		pop |= node->onPop();
	}
	return pop;
}

bool UINode::onPush ()
{
	_texts.clear();
	bool push = true;
	for (UINode* node : _nodes) {
		push |= node->onPush();
	}
	return push;
}

void UINode::addFocus (int32_t x, int32_t y)
{
	_focusMouseX = x;
	_focusMouseY = y;
	if (_focus)
		return;
	_focus = true;
	Log::debug(LOG_UI, "focus for %s", getId().c_str());
	for (const UINodeListenerPtr& listener : _listeners) {
		listener->onAddFocus();
	}
	if (!fequals(_focusAlpha, 0.0f))
		setAlpha(_focusAlpha);
}

void UINode::removeFocus ()
{
	if (!_focus)
		return;
	_mousePressed = false;
	_fingerPressed = false;
	Log::debug(LOG_UI, "remove focus for %s", getId().c_str());
	_focus = false;
	_focusMouseX = -1;
	_focusMouseY = -1;
	for (const UINodeListenerPtr& listener : _listeners) {
		listener->onRemoveFocus();
	}
	if (!fequals(_focusAlpha, 0.0f))
		restoreAlpha();

	for (UINode* nodePtr : _nodes) {
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->removeFocus();
	}
}

bool UINode::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	Log::debug(LOG_UI, "onMouseButtonRelease: %s (%i:%i, %c), enabled: %s", getId().c_str(), x, y, button, (_enabled ? "true" : "false"));
	if (!_enabled)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if (nodePtr->onMouseButtonRelease(x - getRenderX(), y - getRenderY(), button)) {
			return true;
		}
	}

	switch (button) {
	case SDL_BUTTON_LEFT:
		handleDrop(x, y);
		return onMouseLeftRelease(x, y);
	case SDL_BUTTON_RIGHT:
		return onMouseRightRelease(x, y);
	case SDL_BUTTON_MIDDLE:
		return onMouseMiddleRelease(x, y);
	}

	return false;
}

bool UINode::onMultiGesture (float theta, float dist, int32_t numFingers)
{
	if (!_enabled)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onMultiGesture(theta, dist, numFingers)) {
			return true;
		}
	}

	return false;
}

bool UINode::onGesture (int64_t gestureId, float error, int32_t numFingers)
{
	if (!_enabled)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onGesture(gestureId, error, numFingers)) {
			return true;
		}
	}

	return false;
}

bool UINode::onGestureRecord (int64_t gestureId)
{
	if (!_enabled)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onGestureRecord(gestureId)) {
			return true;
		}
	}

	return false;
}

bool UINode::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	Log::debug(LOG_UI, "onMouseButtonPress: %s (%i:%i, %c), enabled: %s", getId().c_str(), x, y, button, (_enabled ? "true" : "false"));
	if (!_enabled)
		return false;

	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onMouseButtonPress(x - getRenderX(), y - getRenderY(), button)) {
			return true;
		}
	}

	switch (button) {
	case SDL_BUTTON_LEFT:
		initDrag(x, y);
		onMouseLeftPress(x, y);
		return true;
	case SDL_BUTTON_RIGHT:
		return onMouseRightPress(x, y);
	case SDL_BUTTON_MIDDLE:
		return onMouseMiddlePress(x, y);
	}

	return false;
}

bool UINode::onMouseLeftRelease (int32_t x, int32_t y)
{
	_mousePressed = false;
	return execute();
}

bool UINode::onControllerButtonPress (int x, int y, const std::string& button)
{
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onControllerButtonPress(x - getRenderX(), y - getRenderY(), button)) {
			return true;
		}
	}
	return false;
}

bool UINode::execute ()
{
	for (const UINodeListenerPtr& listener : _listeners) {
		listener->onClick();
	}

	if (_onActivate.empty()) {
		return !_listeners.empty();
	}

	Commands.executeCommandLine(_onActivate);
	return true;
}

bool UINode::onJoystickButtonPress (int x, int y, uint8_t button)
{
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		if ((*i)->onJoystickButtonPress(x - getRenderX(), y - getRenderY(), button)) {
			return true;
		}
	}
	return false;
}

bool UINode::onMouseMiddleRelease (int32_t x, int32_t y)
{
	return false;
}

bool UINode::onMouseRightRelease (int32_t x, int32_t y)
{
	return false;
}

bool UINode::onMouseMiddlePress (int32_t x, int32_t y)
{
	return false;
}

bool UINode::onMouseRightPress (int32_t x, int32_t y)
{
	return false;
}

bool UINode::onMouseLeftPress (int32_t x, int32_t y)
{
	_mousePressed = true;
	return false;
}

bool UINode::onMouseWheel (int32_t x, int32_t y)
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

void UINode::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (!nodePtr->hasFocus())
			continue;
		nodePtr->onMouseMotion(x - getRenderX(), y - getRenderY(), relX, relY);
	}
}

bool UINode::onJoystickMotion (bool horizontal, int value)
{
	for (UINodeListRevIter i = _nodes.rbegin(); i != _nodes.rend(); ++i) {
		UINode* nodePtr = *i;
		if (nodePtr->onJoystickMotion(horizontal, value))
			return true;
	}
	return false;
}

bool UINode::checkBounds (int x, int y) const
{
	const float _x = x / static_cast<float>(_frontend->getWidth());
	const float _y = y / static_cast<float>(_frontend->getHeight());
	return checkAABB(_x, _y, getX(), getY(), getWidth(), getHeight());
}

void UINode::addBefore (UINode* reference, UINode* node)
{
	UINodeListIter i = std::find(_nodes.begin(), _nodes.end(), reference);
	if (i == _nodes.end()) {
		add(node);
		return;
	}

	if (i == _nodes.begin()) {
		addFront(node);
		return;
	}

	_nodes.insert(i, node);

	if (_layout)
		_layout->addNode(node);

	node->setParent(this);
	node->onAdd();
}

void UINode::addFront (UINode* node)
{
	_nodes.insert(_nodes.begin(), node);

	if (_layout)
		_layout->addNode(node);

	node->setParent(this);
	node->onAdd();
}

void UINode::add (UINode* node)
{
	_nodes.push_back(node);

	if (_layout)
		_layout->addNode(node);

	node->setParent(this);
	node->onAdd();
}

UINode* UINode::getNode (const std::string& nodeId)
{
	if (_nodes.empty())
		return nullptr;

	for (UINode* nodePtr : _nodes) {
		if (nodePtr->getId() == nodeId)
			return nodePtr;

		UINode* nodeR = nodePtr->getNode(nodeId);
		if (nodeR)
			return nodeR;
	}

	return nullptr;
}

void UINode::setTooltip (const std::string& tooltip)
{
	_tooltip = tooltip;
}

void UINode::setLayout (IUILayout* layout)
{
	_layout = layout;
	for (UINode* nodePtr : _nodes) {
		_layout->addNode(nodePtr);
	}
}

void UINode::setVisible (bool visible)
{
	if (!visible)
		removeFocus();
	_visible = visible;
}
