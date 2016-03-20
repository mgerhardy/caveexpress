#pragma once

#include <memory>
#include "common/NonCopyable.h"
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdint.h>

class FontChar {
private:
	int character;
	int width;
	int x;
	int y;
	int w;
	int h;
	int ox;
	int oy;
	float widthFactor;
	float heightFactor;
public:
	FontChar (int _character, int _width, int _x, int _y, int _w, int _h, int _ox, int _oy) :
			character(_character), width(_width), x(_x), y(_y), w(_w), h(_h), ox(_ox), oy(_oy), widthFactor(1.0f), heightFactor(
					1.0f)
	{
	}

	FontChar () :
			character('\0'), width(-1), x(-1), y(-1), w(-1), h(-1), ox(-1), oy(-1), widthFactor(1.0f), heightFactor(1.0f)
	{
	}

	inline void setWidthFactor (float _widthFactor)
	{
		widthFactor = _widthFactor;
	}
	inline void setHeightFactor (float _heightFactor)
	{
		heightFactor = _heightFactor;
	}
	inline int getCharacter () const
	{
		return character;
	}
	inline bool operator()() const
	{
		return character != '\0';
	}
	inline int getWidth () const
	{
		const float nwidth = width * widthFactor;
		return nwidth + 0.5f;
	}
	inline int getX () const
	{
		const float nx = x * widthFactor;
		return nx;
	}
	inline int getY () const
	{
		const float ny = y * heightFactor;
		return ny;
	}
	inline int getW () const
	{
		const float nw = w * widthFactor;
		return nw + 0.5f;
	}
	inline int getH () const
	{
		const float nh = h * heightFactor;
		return nh + 0.5f;
	}
	inline int getOX () const
	{
		const float nox = ox * widthFactor;
		return nox;
	}
	inline int getOY () const
	{
		const float noy = oy * heightFactor;
		return noy;
	}
};

typedef std::vector<FontChar> FontChars;

class FontDef {
public:
	// the id if the fontdef
	const std::string id;
	int textureWidth;
	int textureHeight;
	std::string textureName;

	FontDef (const std::string& _id, int height, int metricsHeight, int metricsAscender, int metricsDescender) :
			id(_id), textureWidth(0), textureHeight(0), textureName(""), _height(height), _metricsHeight(metricsHeight), _metricsAscender(
					metricsAscender), _metricsDescender(metricsDescender), _heightFactor(1.0f), _widthFactor(1.0f)
	{
	}

	inline int getHeight () const
	{
		const float newHeight = _height * _heightFactor;
		return newHeight + 0.5f;
	}

	inline int getMetricsHeight () const
	{
		return _metricsHeight * _heightFactor;
	}

	inline int getMetricsAscender () const
	{
		return _metricsAscender * _heightFactor;
	}

	inline int getMetricsDescender () const
	{
		return _metricsDescender * _heightFactor;
	}

	const FontChar* getFontChar (int character);

	void updateChars (int textureWidth, int textureHeight);
	void init (const FontChars& fontChars);

private:
	std::unordered_map<int, FontChar> _fontCharMap;
	int _height;
	int _metricsHeight;
	int _metricsAscender;
	int _metricsDescender;

	float _heightFactor;
	float _widthFactor;
};

typedef std::shared_ptr<FontDef> FontDefPtr;
typedef std::map<std::string, FontDefPtr> FontDefMap;
typedef FontDefMap::const_iterator FontDefMapConstIter;

class FontDefinition {
private:
	FontDefMap _fontDefs;
public:
	FontDefinition();

	FontDefPtr getFontDefinition (const std::string& fontName) const;

	FontDefMapConstIter begin () const
	{
		return _fontDefs.begin();
	}

	FontDefMapConstIter end () const
	{
		return _fontDefs.end();
	}

	virtual ~FontDefinition() {}
};
