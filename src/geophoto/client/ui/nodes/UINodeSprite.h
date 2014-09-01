#pragma once

#include "UINode.h"
#include "client/sprites/Sprite.h"

class UINodeSprite: public UINode {
private:
	typedef std::vector<SpritePtr> SpriteVector;
	SpriteVector _sprites;
	int _offset;
	float _borderWidth;
	float _borderHeight;
	int _spriteWidth;
	int _spriteHeight;
	float _movementXEnd;
	float _movementYEnd;
	float _movementSpeed;
	bool _movementActive;
public:
	UINodeSprite (IFrontend *frontend, int spriteWidth = -1, int spriteHeight = -1);
	virtual ~UINodeSprite ();

	void addSprite (const SpritePtr& sprite);
	void clearSprites ();
	void setSpriteOffset (int offset);
	void setAspectRatioSize (float width, float height, float upScaleFactor = 1.0f);
	void alignToMiddle ();
	// speed is the amount of pixels in the range 0.0-1.0 that the sprite will move in one millisecond
	void setMovement (float xStart, float yStart, float xEnd, float yEnd, float speed = 0.0004f);
	bool isMovementActive () const;

	// UINode
	void setPos (float x, float y);
	float getAutoWidth () const override;
	float getAutoHeight () const override;
	void render (int x, int y) const override;
	void update (uint32_t deltaTime) override;
};

inline void UINodeSprite::addSprite (const SpritePtr& sprite)
{
	_sprites.push_back(sprite);
	autoSize();
}

inline void UINodeSprite::clearSprites ()
{
	_sprites.clear();
}

inline void UINodeSprite::setSpriteOffset (int offset)
{
	_offset = offset;
}
