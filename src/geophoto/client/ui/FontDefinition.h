#pragma once

#include "common/Pointers.h"
#include "common/NonCopyable.h"
#include <map>
#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>

struct FontChar {
	std::string character;
	int width;
	int x;
	int y;
	int w;
	int h;
	int ox;
	int oy;
};

inline std::ostream& operator<< (std::ostream &stream, const FontChar &in)
{
	stream << "(character: " << in.character << ", width: " << in.width;
	stream << ", x: " << in.x << ", y: " << in.y << ", w: " << in.w << ", h: ";
	stream << in.h << ", ox: " << in.ox << ", oy: " << in.oy << ")";
	return stream;
}

class FontDef {
private:
	std::map<std::string, FontChar*> _fontCharMap;
public:
	FontDef(const std::string& _id) :
			id(_id), height(0), textureHeight(0), textureWidth(0), textureName(
					""), metricsAscender(0), metricsDescender(0), metricsHeight(0) {
	}

	FontDef &operator= (const FontDef &r)
	{
		return *this;
	}

	// the id if the fontdef
	const std::string id;
	int height;
	int textureWidth;
	int textureHeight;
	std::string textureName;

	int metricsHeight;
	int metricsAscender;
	int metricsDescender;

	std::vector<FontChar> fontChars;

	const FontChar* getFontChar (char character);

	void updateChars (int textureWidth, int textureHeight);
};

inline std::ostream& operator<< (std::ostream &stream, const FontDef &in)
{
	stream << "(id: " << in.id << ", height: " << in.height;
	stream << ", textureWidth: " << in.textureWidth << ", v: ";
	stream << in.textureHeight << ", textureName: " << in.textureName;
	stream << ", metricsHeight: " << in.metricsHeight << ", metricsAscender: ";
	stream << in.metricsAscender << ", metricsDescender: ";
	stream << in.metricsDescender << ", chars: " << in.fontChars.size() << ")";
	return stream;
}

typedef SharedPtr<FontDef> FontDefPtr;
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
