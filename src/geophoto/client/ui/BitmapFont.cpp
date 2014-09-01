#include "BitmapFont.h"
#include "shared/IFrontend.h"
#include "client/ui/UI.h"
#include "shared/Logger.h"
#include "common/System.h"

BitmapFont::BitmapFont(const FontDefPtr& fontDefPtr, IFrontend *frontend) :
		_frontend(frontend),_fontDefPtr(fontDefPtr) {
	_font = UI::get().loadTexture(fontDefPtr->textureName);
	if (!_font || !_font->isValid()) {
		System.exit("invalid font definition with texture " + fontDefPtr->textureName, 1);
	}
	_fontDefPtr->updateChars(_font->getWidth(), _font->getHeight());
}

BitmapFont::~BitmapFont (void)
{
}

int BitmapFont::getCharWidth () const
{
	// TODO:
	return -1;
}

int BitmapFont::getTextWidth (const std::string& string) const
{
	int width = 0;
	int lineWidth = 0;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i) {
		const unsigned char chr = *i;
		if (chr == '\n') {
			width = std::max(lineWidth, width);
			lineWidth = 0;
			continue;
		}
		const FontChar* fontChr = _fontDefPtr->getFontChar(chr);
		if (fontChr == nullptr)
			continue;
		lineWidth += fontChr->width;
	}
	return std::max(lineWidth, width);
}

void BitmapFont::print (const std::string& text, const Color& color, int x, int y) const
{
	const int beginX = x;

	_frontend->setColor(color);
	const TextureRect sourceRect = _font->getSourceRect();
	for (std::string::const_iterator i = text.begin(); i != text.end(); ++i) {
		const unsigned char chr = *i;
		if (chr == '\n') {
			x = beginX;
			y += _fontDefPtr->height;
			continue;
		}
		const FontChar* fontChr = _fontDefPtr->getFontChar(chr);
		if (fontChr == nullptr)
			continue;
		_font->setRect(sourceRect.x + fontChr->x, sourceRect.y + fontChr->y, fontChr->w, fontChr->h);
		_frontend->renderImage(_font.get(), x + fontChr->ox, y + _fontDefPtr->height - fontChr->oy, fontChr->w, fontChr->h, 0, color[3]);
		x += fontChr->width;
	}
	_font->setRect(sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h);
	_frontend->resetColor();
}
