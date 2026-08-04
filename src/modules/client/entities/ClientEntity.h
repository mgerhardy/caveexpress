#pragma once

#include "ClientEntityFactory.h"
#include "common/Animation.h"
#include "common/Direction.h"
#include "common/EntityAlignment.h"
#include "common/ThemeType.h"
#include "sprites/Sprite.h"
#include "common/SpriteDefinition.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <algorithm>

class ClientEntity;
typedef ClientEntity* ClientEntityPtr;

class ClientEntity {
protected:
	ClientEntity (const EntityType& type, uint16_t id, float x, float y, float sizeX,
			float sizeY, const SoundMapping& soundMappingClientEntity, EntityAlignment align, EntityAngle angle);

	virtual void onVisibilityChanged ();
	void renderDot (IFrontend* frontend, int x, int y, const Color& color = colorRed) const;
	void renderOverlays(IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY, int posX, int posY, int mapPixelWidth, int mapPixelHeight) const;

	void calcPosition(const Layer &layer, int scale, float zoom, int &posX, int &posY) const;

	// calculate the entity overlay offset
	void calcOffset(int scale, float zoom, int posX, int posY, int &offsetPosX, int &offsetPosY) const;
public:
	class Factory: public IClientEntityFactory {
		ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const override;
	};
	static Factory FACTORY;
	virtual ~ClientEntity ();

	void remove();
	const SpritePtr& getSprite () const;
	SpritePtr& getSprite ();
	const vec2& getPos () const;
	const vec2& getSize () const;
	/**
	 * @param[in] animateSpriteAlways if false, multi-frame sprites only advance while a timed move lerp is active
	 */
	virtual bool update (uint32_t deltaTime, bool lerpPos, bool animateSpriteAlways = true);

	/** When true and the map uses timed grid lerps, walk frames only advance while sliding between cells. */
	virtual bool animateSpriteOnlyWhenMoving () const { return false; }
	virtual void setAnimationType (const Animation& type);
	void setThemeType (const ThemeType& theme);
	virtual std::string getSpriteName() const;
	/**
	 * @param[in] lerp whether to interpolate toward @c pos
	 * @param[in] durationMillis time-based lerp duration; 0 keeps the legacy frame-ratio lerp
	 */
	void setPos (const vec2& pos, bool lerp, uint32_t durationMillis = 0);
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
	virtual void render(IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX,
						int offsetY, int mapPixelWidth, int mapPixelHeight) const;

	inline const Animation& getAnimation () const
	{
		return *_animation;
	}

	void addOverlay (const SpritePtr& sprite, int offsetX = 0, int offsetY = 0);
	void removeOverlay (const SpritePtr& sprite);

	void addRope (const ClientEntityPtr& ropeEntity)
	{
		_ropeEntity = ropeEntity;
	}

	void removeRope ()
	{
		_ropeEntity = nullptr;
	}

	EntityAlignment getAlignment () const
	{
		return _align;
	}

protected:
	// used for lerping the position
	vec2 _nextPos;
	// used for lerping the position
	vec2 _prevPos;
	// time-based lerp (durationMillis > 0 in setPos); 0 means legacy frame-ratio lerp
	uint32_t _lerpDuration;
	uint32_t _lerpElapsed;
	// the size of the entity. This might differ from what the physical size is in the server
	vec2 _size;
	const EntityType &_type;
	// the unique id of this entity
	uint16_t _id;

	struct Overlay {
		SpritePtr sprite;
		int offsetX;
		int offsetY;

		Overlay (const SpritePtr& _sprite, int _offsetX, int _offsetY) :
				sprite(_sprite), offsetX(_offsetX), offsetY(_offsetY)
		{
		}
	};

	typedef std::vector<Overlay> EntityOverlays;
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
	typedef std::unordered_map<std::string, SpritePtr> SpritesMap;
	typedef SpritesMap::const_iterator SpritesMapConstIter;
	SpritesMap _sprites;
	uint8_t _state;
	// the current animation to play
	const Animation *_animation;
	const ThemeType *_theme;
	// initialized a delayed removal of the entity until it is faded out
	// this entity is already removed on the server side
	uint32_t _fadeOutTime;
	// the alpha value is 1.0 by default and decreases if the fade out was initialized
	float _alpha;
	// the entity that is connected with this entity via the rope
	ClientEntityPtr _ropeEntity;

	int _animationSound;
	SoundMapping _soundMapping;
	bool _visible;
	mutable bool _visChanged;
	EntityAlignment _align;

	mutable int _screenPosX;
	mutable int _screenPosY;

	mutable int _screenWidth;
	mutable int _screenHeight;

	TexturePtr _ropeTexture;
};

inline void ClientEntity::removeOverlay (const SpritePtr& sprite)
{
	auto pred = [&sprite](const Overlay &lhs) {
		return lhs.sprite == sprite;
	};
	auto i = std::find_if(_entityOverlays.begin(), _entityOverlays.end(), pred);
	if (i != _entityOverlays.end())
		_entityOverlays.erase(i);
}

inline void ClientEntity::addOverlay (const SpritePtr& sprite, int offsetX, int offsetY)
{
	if (!sprite)
		return;
	_entityOverlays.emplace_back(sprite, offsetX, offsetY);
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
