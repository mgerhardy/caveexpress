#include "UINode.h"
#include "client/ui/UI.h"
#include "shared/CommandSystem.h"
#include "shared/ConfigManager.h"
#include "shared/Logger.h"
#include "shared/IFrontend.h"
#include "shared/Payment.h"
#include "client/ui/BitmapFont.h"
#include <SDL.h>

int UINode::_counter = 0;

UINode::UINode (IFrontend *frontend, const std::string& id) :
		_padding(0.0f), _onActivate(""), _focus(false), _focusAlpha(0.0f), _focusMouseX(-1), _focusMouseY(-1), _visible(
				true), _enabled(true), _dragStartX(-1), _dragStartY(-1), _alpha(1.0f), _previousAlpha(1.0f), _id(id), _renderBorder(
				false), _frontend(frontend), _align(NODE_ALIGN_LEFT), _time(0)
{
	setPos(0.0f, 0.0f);
	setSize(0.0f, 0.0f);
	Vector4Set(colorWhite, _borderColor);
	Vector4Set(colorNull, _backgroundColor);

	if (_id.empty())
		_id = string::toString(_counter++);
}

UINode::~UINode ()
{
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

void UINode::setPos (float x, float y)
{
	_pos.x = x;
	_pos.y = y;
	updateAlignment();
}

void UINode::setSize (float w, float h)
{
	_size.x = w;
	_size.y = h;
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
	_texture = loadTexture(texture);
	if (!hasImage())
		return _texture;

	float w = getWidth();
	if (w <= EPSILON) {
		w = getAutoWidth();
	}
	float h = getHeight();
	if (h <= EPSILON) {
		h = getAutoHeight();
	}
	setSize(w, h);
	updateAlignment();
	return _texture;
}

void UINode::update (uint32_t deltaTime)
{
	_time += deltaTime;
	if (_text.delayMillis > 0) {
		_text.delayMillis -= std::min(_text.delayMillis, deltaTime);
	}
}

void UINode::updateAlignment ()
{
	int x;
	int y;
	if (_align & NODE_ALIGN_CENTER) {
		x = _frontend->getWidth() / 2 - getRenderWidth(false) / 2;
	} else if (_align & NODE_ALIGN_RIGHT) {
		x = _frontend->getWidth() - getRenderWidth(false);
	} else {
		// default is left
		x = getRenderX(false);
	}

	if (_align & NODE_ALIGN_MIDDLE) {
		y = _frontend->getHeight() / 2 - getRenderHeight(false) / 2;
	} else if (_align & NODE_ALIGN_BOTTOM) {
		y = _frontend->getHeight() - getRenderHeight(false);
	} else {
		// default is top
		y = getRenderY(false);
	}

	_pos.x = x / static_cast<float>(_frontend->getWidth());
	_pos.y = y / static_cast<float>(_frontend->getHeight());
}

void UINode::displayText (const std::string& text, uint32_t delayMillis, float x, float y)
{
	o(String::format("Display text '" + text + "' for " + string::toString(delayMillis) + "ms"));
	_text.set(text, delayMillis, x, y);
}

void UINode::alignTo (const UINode* node, int align, float padding)
{
	int x;
	int y;
	if (align & NODE_ALIGN_CENTER) {
		x = node->getRenderCenterX() - getRenderWidth() / 2;
	} else if (align & NODE_ALIGN_RIGHT) {
		x = (node->getRenderX() + node->getRenderWidth()) - getRenderWidth(false) - padding * _frontend->getWidth();
	} else {
		// default is left
		x = node->getRenderX() + padding * _frontend->getWidth();
	}

	if (align & NODE_ALIGN_MIDDLE) {
		y = node->getRenderCenterY() - getRenderHeight() / 2;
	} else if (align & NODE_ALIGN_BOTTOM) {
		y = (node->getRenderY() + node->getRenderHeight()) - getRenderHeight(false) - padding * _frontend->getWidth();
	} else {
		// default is top
		y = node->getRenderY() + padding * _frontend->getWidth();
	}

	setPos(x / static_cast<float>(_frontend->getWidth()), y / static_cast<float>(_frontend->getHeight()));
}

void UINode::render (int x, int y) const
{
	const int w = getRenderWidth(false);
	const int h = getRenderHeight(false);
	if (_backgroundColor[3] > 0.001f)
		renderFilledRect(x + getRenderX(false), y + getRenderY(false), w, h, _backgroundColor);

	if (_texture)
		renderImage(_texture, x + getRenderX(), y + getRenderY(), getRenderWidth(), getRenderHeight(), _alpha);

	if (_renderBorder)
		renderRect(x + getRenderX(false), y + getRenderY(false), w, h, _borderColor);

	if (_text.delayMillis > 0) {
		const BitmapFontPtr& font = getFont(HUGE_FONT);
		const int fontX = _text.pos.x > 0.0f ? _text.pos.x * _frontend->getWidth() : getRenderCenterX() - font->getTextWidth(_text.text) / 2.0f;
		const int fontY = _text.pos.y > 0.0f ? _text.pos.y * _frontend->getHeight() : getRenderCenterY() - font->getTextHeight(_text.text) / 2.0f;
		font->print(_text.text, colorWhite, fontX, fontY);
	}

	if (hasFocus()) {
		const bool debug = Config.getConfigVar("debugui")->getBoolValue();
		if (debug) {
			renderRect(x + getRenderX(false), y + getRenderY(false), w, h, colorGreen);
			renderFilledRect(getRenderCenterX(), getRenderCenterY(), 4, 4, colorRed);
			renderFilledRect(getRenderX(), getRenderY(), 4, 4, colorBlue);
		}
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
	if (!texture)
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
	initDrag(x, y);
	return false;
}

void UINode::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
}

bool UINode::onFingerRelease (int64_t finger, uint16_t x, uint16_t y)
{
	return execute(x, y);
}

bool UINode::onKeyPress (int32_t key, int16_t modifier)
{
	return false;
}

bool UINode::onKeyRelease (int32_t key)
{
	return false;
}

bool UINode::onPop ()
{
	return true;
}

bool UINode::onPush ()
{
	return true;
}

bool UINode::hasMultipleFocus ()
{
	return false;
}

void UINode::addFocus (int32_t x, int32_t y)
{
	if (x == -1)
		x = getCenter() * _frontend->getWidth();
	if (y == -1)
		x = getMiddle() * _frontend->getHeight();
	_focusMouseX = x;
	_focusMouseY = y;
	if (_focus)
		return;
	_focus = true;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onAddFocus();
	}
	if (!fequals(_focusAlpha, 0.0f))
		setAlpha(_focusAlpha);
}

void UINode::removeFocus ()
{
	_focus = false;
	_focusMouseX = -1;
	_focusMouseY = -1;
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onRemoveFocus();
	}
	if (!fequals(_focusAlpha, 0.0f))
		restoreAlpha();
}

bool UINode::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!_enabled)
		return false;

	switch (button) {
	case SDL_BUTTON_LEFT:
		return onMouseLeftRelease(x, y);
	case SDL_BUTTON_RIGHT:
		return onMouseRightRelease(x, y);
	case SDL_BUTTON_MIDDLE:
		return onMouseMiddleRelease(x, y);
	}

	return false;
}

bool UINode::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	if (!_enabled)
		return false;

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
	return execute(x, y);
}

bool UINode::onControllerButtonPress (int x, int y, const std::string& button)
{
	if (!_enabled)
		return false;

	return execute(x, y);
}

bool UINode::execute (int x, int y)
{
	if (x == -1)
		x = getCenter() * _frontend->getWidth();
	if (y == -1)
		x = getMiddle() * _frontend->getHeight();

	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onClick();
	}

	handleDrop(x, y);

	if (_onActivate.empty())
		return false;

	Commands.executeCommandLine(_onActivate);
	return true;
}

bool UINode::onJoystickButtonPress (int x, int y, uint8_t button)
{
	if (!_enabled)
		return false;

	return execute(x, y);
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
	return false;
}

bool UINode::onMouseWheel (int32_t x, int32_t y)
{
	return false;
}

bool UINode::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	return false;
}

bool UINode::onJoystickMotion (bool horizontal, int value)
{
	return false;
}

bool UINode::wantFocus () const
{
	return false;
}

bool UINode::checkBounds (int x, int y) const
{
	const float _x = x / static_cast<float>(_frontend->getWidth());
	const float _y = y / static_cast<float>(_frontend->getHeight());
	return checkAABB(_x, _y, getX(), getY(), getWidth(), getHeight());
}

UINode* UINode::getNode (const std::string& nodeId)
{
	if (nodeId == getId())
		return this;

	return nullptr;
}
