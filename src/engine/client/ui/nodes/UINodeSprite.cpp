#include "UINodeSprite.h"
#include "engine/common/Logger.h"
#include <cassert>

UINodeSprite::UINodeSprite (IFrontend *frontend, int spriteWidth, int spriteHeight) :
		UINode(frontend), _offset(0), _borderWidth(-1.0f), _borderHeight(-1.0f), _spriteWidth(
				spriteWidth), _spriteHeight(spriteHeight), _movementXEnd(0.0f), _movementYEnd(
				0.0f), _movementSpeed(0.0f), _movementActive(false) {
}

UINodeSprite::~UINodeSprite ()
{
}

void UINodeSprite::setMovement (float xStart, float yStart, float xEnd, float yEnd, float speed)
{
	setPos(xStart, yStart);
	_movementXEnd = xEnd;
	_movementYEnd = yEnd;
	_movementSpeed = speed;
	_movementActive = true;
}

bool UINodeSprite::isMovementActive () const
{
	return _movementActive;
}

void UINodeSprite::render (int x, int y) const
{
	assert(_spriteWidth != -1);
	assert(_spriteHeight != -1);
	UINode::render(x, y);
	int n = 0;
	const int baseSpriteX = x + getRenderX(true);
	const int spriteY = y + getRenderY(true);
	for (SpriteVector::const_iterator i = _sprites.begin(); i != _sprites.end(); ++i, ++n) {
		const int spriteX = n * _offset + baseSpriteX;
		for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
			(*i)->render(_frontend, layer, spriteX, spriteY, _spriteWidth, _spriteHeight, (uint16_t)0, _alpha);
		}
	}
}

void UINodeSprite::update (uint32_t deltaTime)
{
	UINode::update(deltaTime);
	if (_movementActive) {
		const float dX = _movementXEnd - getX();
		const float dY = _movementYEnd - getY();
		const float goalDist = sqrt((dX * dX) + (dY * dY));
		const float v = std::abs(_movementSpeed * deltaTime);
		if (goalDist > v) {
			const float ratio = v / goalDist;
			const float xMove = ratio * dX;
			const float yMove = ratio * dY;
			_pos.x += xMove;
			_pos.y += yMove;
		} else {
			_pos.x = _movementXEnd;
			_pos.y = _movementYEnd;
			_movementActive = false;
		}
	}
	for (SpriteVector::iterator i = _sprites.begin(); i != _sprites.end(); ++i) {
		(*i)->update(deltaTime);
	}
}

void UINodeSprite::setPos (float x, float y)
{
	_pos.x = x;
	_pos.y = y;
}

void UINodeSprite::setAspectRatioSize (float width, float height, float upScaleFactor)
{
	// save the old values
	_borderWidth = getWidth();
	_borderHeight = getHeight();
	autoSize();
	float spriteW = getWidth();
	float spriteH = getHeight();
	const float aspectW = spriteW / static_cast<float>(width);
	const float aspectH = spriteH / static_cast<float>(height);
	float widthF;
	float heightF;
	if (aspectW > aspectH) {
		widthF = spriteW / aspectW;
		heightF = spriteH / aspectW;
	} else {
		widthF = spriteW / aspectH;
		heightF = spriteH / aspectH;
	}

	setSize(widthF, heightF);
	_spriteWidth = widthF * _frontend->getWidth();
	_spriteHeight = heightF * _frontend->getHeight();

	if (upScaleFactor <= 1.0f)
		return;

	spriteW *= upScaleFactor;
	spriteH *= upScaleFactor;

	const float w = getWidth();
	const float h = getHeight();
	if (w > spriteW) {
		const float ratio = w / static_cast<float>(spriteW);
		setSize(w / ratio, h / ratio);
		_spriteWidth = w / ratio * _frontend->getWidth();
		_spriteHeight = h / ratio * _frontend->getHeight();
	}

	if (h > spriteH) {
		const float ratio = h / static_cast<float>(spriteH);
		setSize(w / ratio, h / ratio);
		_spriteWidth = w / ratio * _frontend->getWidth();
		_spriteHeight = h / ratio * _frontend->getHeight();
	}
}

float UINodeSprite::getAutoWidth () const
{
	if (_sprites.empty())
		return 0.0f;

	const int w = _spriteWidth == -1 ? _sprites.front()->getMaxWidth() : _spriteWidth;
	return _sprites.size() * (w + _offset) / static_cast<float>(_frontend->getWidth());
}

float UINodeSprite::getAutoHeight () const
{
	if (_sprites.empty())
		return 0.0f;

	const int h = _spriteHeight == -1 ? _sprites.front()->getMaxHeight() : _spriteHeight;
	return h / static_cast<float>(_frontend->getHeight());
}

void UINodeSprite::alignToMiddle ()
{
	if (_borderWidth <= 0.0f || _borderHeight <= 0.0f)
		return;

	const float deltaX = (_borderWidth - getWidth()) / 2.0f;
	const float deltaY = (_borderHeight - getHeight()) / 2.0f;
	setPos(getX() + deltaX, getY() + deltaY);
}
