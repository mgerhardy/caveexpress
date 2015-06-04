#pragma once

#include "ClientEntityFactory.h"
#include "common/Animation.h"
#include "common/Direction.h"
#include "common/EntityAlignment.h"
#include "sprites/Sprite.h"
#include "common/SpriteDefinition.h"
#include "common/Pointers.h"
#include <Box2D/Common/b2Math.h>
#include <map>
#include <string>

class ClientEntity;
typedef ClientEntity* ClientEntityPtr;

class ClientEntity {
protected:
	SoundMapping _soundMapping;

	ClientEntity (const EntityType& type, uint16_t id, float x, float y, float sizeX,
			float sizeY, const SoundMapping& soundMappingClientEntity, EntityAlignment align, EntityAngle angle);

	virtual void onVisibilityChanged ();
	void renderDot (IFrontend* frontend, int x, int y, const Color& color = colorRed) const;
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const;
	};
	static Factory FACTORY;
	virtual ~ClientEntity ();

	void remove();
	const SpritePtr& getSprite () const;
	SpritePtr& getSprite ();
	const vec2& getPos () const;
	const vec2& getSize () const;
	virtual bool update (uint32_t deltaTime, bool lerpPos);
	void setAnimationType (const Animation& type);
	void setPos (const vec2& pos, bool lerp);
	Direction getMoveDirection ();
	void setAngle (int16_t angle);
	void initFadeOut ();
	uint16_t getID () const;

	inline const EntityType& getType () const
	{
		return _type;
	}

	virtual void changeState (uint8_t state)
	{
		_state = state;
	}

	inline void setScreenSize (int width, int height) const
	{
		_screenWidth = width;
		_screenHeight = height;
	}

	inline void setAlpha (float alpha)
	{
		_alpha = alpha;
	}

	inline void getScreenSize (int& width, int& height) const
	{
		width = _screenWidth;
		height = _screenHeight;
	}

	inline void setScreenPos (int x, int y) const
	{
		_screenPosX = x;
		_screenPosY = y;
	}

	inline void getScreenPos (int& x, int& y) const
	{
		x = _screenPosX;
		y = _screenPosY;
	}

	inline uint8_t getState () const
	{
		return _state;
	}

	inline void setAnimationSound (int animationSound)
	{
		_animationSound = animationSound;
	}

	// @param[in] scale The conversion from the physics coordinate system to the pixel coordinate system.
	virtual void render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX = 0, int offsetY = 0) const;

	inline const Animation& getAnimation () const
	{
		return *_animation;
	}

	void addOverlay (const SpritePtr& sprite);

	void addRope (const ClientEntityPtr& ropeEntity)
	{
		_ropeEntity = ropeEntity;
	}

	void removeRope ()
	{
		_ropeEntity = nullptr;
	}

protected:
	// used for lerping the position
	vec2 _nextPos;
	// used for lerping the position
	vec2 _prevPos;
	// the size of the entity. This might differ from what the physical size is in the server
	vec2 _size;
	const EntityType &_type;
	// the unique id of this entity
	uint16_t _id;

	typedef std::vector<SpritePtr> EntityOverlays;
	typedef EntityOverlays::iterator EntityOverlaysIter;
	typedef EntityOverlays::const_iterator EntityOverlaysConstIter;
	EntityOverlays _entityOverlays;

	// angle in degrees
	EntityAngle _angle;
	// the position of the entity. This position is lerped in the update of the entity and thus might be a little bit
	// different from what we have in the server as position for this entity.
	// Note: this is the server position and does not have any pixel meaning - multiplicate with the map scale value
	// to get the pixel position
	vec2 _pos;
	// the lifetime of this entity
	uint32_t _time;
	mutable SpritePtr _currSprite;
	typedef std::map<const Animation*, SpritePtr> SpritesMap;
	typedef SpritesMap::const_iterator SpritesMapConstIter;
	SpritesMap _sprites;
	uint8_t _state;
	// the current animation to play
	const Animation *_animation;
	// initialized a delayed removal of the entity until it is faded out
	// this entity is already removed on the server side
	uint32_t _fadeOutTime;
	// the alpha value is 1.0 by default and decreases if the fade out was initialized
	float _alpha;
	// the entity that is connected with this entity via the rope
	ClientEntityPtr _ropeEntity;

	int _animationSound;
	bool _visible;
	mutable bool _visChanged;
	EntityAlignment _align;

	mutable int _screenPosX;
	mutable int _screenPosY;

	mutable int _screenWidth;
	mutable int _screenHeight;
};

inline void ClientEntity::addOverlay (const SpritePtr& sprite)
{
	_entityOverlays.push_back(sprite);
}

inline void ClientEntity::initFadeOut ()
{
	_fadeOutTime = std::max(1U, _time);
}

inline void ClientEntity::setAngle (int16_t angle)
{
	_angle = angle;
}

inline const SpritePtr& ClientEntity::getSprite () const
{
	return _currSprite;
}

inline SpritePtr& ClientEntity::getSprite ()
{
	return _currSprite;
}

inline const vec2& ClientEntity::getPos () const
{
	return _pos;
}

inline const vec2& ClientEntity::getSize () const
{
	return _size;
}

inline uint16_t ClientEntity::getID () const
{
	return _id;
}
