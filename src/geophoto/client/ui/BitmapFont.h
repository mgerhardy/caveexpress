#pragma once

#include "client/textures/Texture.h"
#include "client/ui/FontDefinition.h"
#include <algorithm>

class IFrontend;

class BitmapFont {
	mutable TexturePtr _font;
	IFrontend *_frontend;
	TextureRect _rect;
	FontDefPtr _fontDefPtr;
public:
	BitmapFont (const FontDefPtr& fontDefPtr, IFrontend *frontend);
	virtual ~BitmapFont (void);
	void print (const std::string& text, const Color& color, int x, int y) const;
	int getCharHeight () const;
	int getCharWidth () const;
	int getTextWidth (const std::string& string) const;
	int getTextHeight (const std::string& string) const;
};

inline int BitmapFont::getTextHeight (const std::string& string) const
{
	const int lines = 1 + std::count(string.begin(), string.end(), '\n');
	return getCharHeight() * lines;
}

inline int BitmapFont::getCharHeight () const
{
	return _fontDefPtr->metricsHeight;
}
