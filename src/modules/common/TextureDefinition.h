#pragma once

#include "common/NonCopyable.h"
#include "common/Math.h"
#include "common/Compiler.h"
#include <map>
#include <string>

namespace {
const std::string TEXTURE_DIRECTION = "DIR";
const std::string TEXTURE_DIRECTION_RIGHT = "right";
const std::string TEXTURE_DIRECTION_LEFT = "left";
}

class IProgressCallback;

struct TextureDefinitionTrim {
	TextureDefinitionTrim() :
			trimmedWidth(0), trimmedHeight(0), untrimmedWidth(0), untrimmedHeight(
					0), trimmedOffsetX(0), trimmedOffsetY(0) {
	}
	TextureDefinitionTrim(int _trimmedWidth, int _trimmedHeight,
			int _untrimmedWidth, int _untrimmedHeight, int _trimmedOffsetX,
			int _trimmedOffsetY) :
			trimmedWidth(_trimmedWidth), trimmedHeight(_trimmedHeight), untrimmedWidth(
					_untrimmedWidth), untrimmedHeight(_untrimmedHeight), trimmedOffsetX(
					_trimmedOffsetX), trimmedOffsetY(_trimmedOffsetY) {
	}
	int trimmedWidth;
	int trimmedHeight;
	int untrimmedWidth;
	int untrimmedHeight;
	int trimmedOffsetX;
	int trimmedOffsetY;
};

struct TextureDefinitionCoords {
	float x0;
	float y0;
	float x1;
	float y1;

	inline bool isValid () const
	{
		if (x0 < 0.0f || x0 > 1.0f)
			return false;
		if (y0 < 0.0f || y0 > 1.0f)
			return false;
		if (x1 < 0.0f || x1 > 1.0f)
			return false;
		if (y1 < 0.0f || y1 > 1.0f)
			return false;

		if (std::isinf(x0) || std::isnan(x0))
			return false;
		if (std::isinf(y0) || std::isnan(y0))
			return false;
		if (std::isinf(x1) || std::isnan(x1))
			return false;
		if (std::isinf(y1) || std::isnan(y1))
			return false;

		return true;
	}
};

struct TextureDef {
	std::string textureName;
	std::string id;
	TextureDefinitionCoords texcoords;
	TextureDefinitionTrim trim;
	bool mirror;
};

class TextureDefinition: public NonCopyable {
public:
	typedef std::map<std::string, TextureDef> TextureDefMap;
	typedef TextureDefMap::const_iterator TextureDefMapConstIter;

private:
	TextureDefMap _textureDefs;
	TextureDef _emptyTextureDef;
	std::string _textureSize;
	int _cnt;

	inline void create (const std::string& textureName, const std::string& id, const TextureDefinitionCoords& texcoords,
			const TextureDefinitionTrim& trim, bool mirror);
public:
	TextureDefinition (const std::string& textureSize, IProgressCallback* progress = nullptr);
	virtual ~TextureDefinition ();

	inline const TextureDefMap& getMap () const
	{
		return _textureDefs;
	}

	inline int getSize () const
	{
		return _cnt;
	}

	inline const std::string& getTextureSize () const
	{
		return _textureSize;
	}

	bool exists (const std::string& textureName) const;
	const TextureDef& getTextureDef (const std::string& textureName) const;
};
