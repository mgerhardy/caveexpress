#pragma once

#include "engine/common/Animation.h"
#include "engine/common/EntityType.h"
#include "engine/common/SpriteType.h"
#include "engine/common/ThemeType.h"
#include "engine/common/Layer.h"
#include "engine/common/IMap.h"
#include "engine/common/Pointers.h"
#include "engine/common/NonCopyable.h"
#include <map>
#include <string>
#include <vector>

typedef int16_t EntityAngle;
class TextureDefinition;

class SpriteDefFrame {
public:
	SpriteDefFrame (const std::string& _name, int _delay, bool _active) :
			name(_name), delay(_delay), active(_active)
	{
	}

	std::string name;
	int delay;
	// this frame can trigger an entity state change depending on the value of this boolean
	bool active;
};

struct SpriteVertex {
	SpriteVertex (float _x, float _y) :
			x(_x), y(_y)
	{
	}

	float x;
	float y;
};

// TODO: b2_maxPolygonVertices
class SpritePolygon {
public:
	typedef std::vector<SpriteVertex> SpriteVector;
	SpriteVector vertices;
	std::string userData;

	SpritePolygon(const std::string& _userData) :
			userData(_userData)
	{
	}
};

class SpriteCircle {
public:
	SpriteVertex center;
	float radius;
	std::string userData;

	SpriteCircle (const std::string& _userData) :
			center(0.0f, 0.0f), radius(0.0f), userData(_userData)
	{
	}
};

class SpriteDef {
private:
	mutable vec2 _cachedShapeSize;
	mutable bool _shapeSizeCalculated;
public:
	SpriteDef (const std::string& _id, const SpriteType& _type, const ThemeType& _theme) :
			_shapeSizeCalculated(false), id(_id), type(_type), theme(_theme), redirect(""), fps(0.0f), width(1.0f), height(
					1.0f), angle(0), rotateable(0.0f), friction(0.2f), restitution(0.0f), delay(0)
	{
	}

	SpriteDef &operator= (const SpriteDef &r)
	{
		return *this;
	}

	// if all the layers do only contain one frame, this is a static sprite with no animation
	inline bool isStatic () const
	{
		for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
			if (textures[layer].size() > 1)
				return false;
		}
		return true;
	}

	inline bool hasShape () const
	{
		return !polygons.empty() || !circles.empty();
	}

	inline bool hasNoTextures () const
	{
		for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
			if (!textures[layer].empty())
				return false;
		}
		return true;
	}

	vec2 calculateSizeFromShapeData () const;
	vec2 getMins () const;
	vec2 getMaxs () const;

	// the id if the spritedef
	const std::string id;

	// the type of the sprite
	const SpriteType& type;

	// the shape data
	std::vector<SpritePolygon> polygons;
	std::vector<SpriteCircle> circles;

	// the theme the tile should be used in
	const ThemeType& theme;

	// some sprites may redirect to other sprites
	std::string redirect;

	// frames per second
	float fps;

	// the width in tiles
	gridSize width;

	// the height in tiles
	gridSize height;

	// initial or fixed rotation in degree (depends on whether the sprite is rotateable)
	EntityAngle angle;

	// the degrees by which this sprite is rotateable
	EntityAngle rotateable;

	float friction;
	float restitution;

	void calcDelay () {
		for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
			SpriteDef::SpriteDefFrames& frames = textures[layer];
			int d = 0;
			for (SpriteDef::TexturesConstIter i = frames.begin(); i != frames.end(); ++i) {
				d += i->delay;
			}
			delay = std::max(delay, d);
		}
	}

	// the frames
	typedef std::vector<SpriteDefFrame> SpriteDefFrames;
	typedef SpriteDefFrames::const_iterator TexturesConstIter;
	typedef SpriteDefFrames::iterator TexturesIter;
	SpriteDefFrames textures[MAX_LAYERS];
	// the overall delay of all the frames
	int delay;
};

typedef SharedPtr<SpriteDef> SpriteDefPtr;
typedef std::map<std::string, SpriteDefPtr> SpriteDefMap;
typedef SpriteDefMap::const_iterator SpriteDefMapConstIter;

// Map contains AnimationType and sprite id name containing
// corresponding animation frames defined in sprites.lua
typedef std::map<Animation, std::string> SpriteIdsMap;
typedef SpriteIdsMap::const_iterator SpriteIdsMapConstIter;

class SpriteDefinition: public NonCopyable {
private:
	SpriteDefMap _spriteDefs;

	SpriteType getType (const std::string& type) const;

	SpriteDefinition ();
public:
	void init (const TextureDefinition& textureDefinition);
	static SpriteDefinition& get ();

	SpriteDefPtr getFromEntityType (const EntityType& entityType, const Animation& animation) const;

	SpriteDefPtr getSpriteDefinition (const std::string& spriteName) const;

	std::string getSpriteName (const EntityType& type, const Animation& animation) const;

	// return the milliseconds that the given animation for the given entity type needs for one full playback
	int getAnimationLength (const EntityType& type, const Animation& animation) const;
	int getAnimationLengthFromDef (const SpriteDefPtr& spriteDef) const;

	bool isFrameActive (uint32_t time, const SpriteDefPtr spriteDef) const;

	SpriteDefMapConstIter begin () const
	{
		return _spriteDefs.begin();
	}

	SpriteDefMapConstIter end () const
	{
		return _spriteDefs.end();
	}

	virtual ~SpriteDefinition ();
};
