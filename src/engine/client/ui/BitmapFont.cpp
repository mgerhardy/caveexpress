#include "BitmapFont.h"
#include "engine/common/IFrontend.h"
#include "engine/client/ui/UI.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"

BitmapFont::BitmapFont(const FontDefPtr& fontDefPtr, IFrontend *frontend) :
		_frontend(frontend),_fontDefPtr(fontDefPtr), _time(0U) {
	_font = UI::get().loadTexture(fontDefPtr->textureName);
	_rand = randBetween(0, 10000);
	if (!_font || !_font->isValid()) {
		System.exit("invalid font definition with texture " + fontDefPtr->textureName, 1);
	}
	_fontDefPtr->updateChars(_font->getWidth(), _font->getHeight());
}

BitmapFont::~BitmapFont (void)
{
	info(LOG_CLIENT, "free the bitmap font " + _fontDefPtr->id);
}

int BitmapFont::getMaxCharsForLength (const std::string& string, int pixelWidth) const
{
	int lineWidth = 0;
	int chars = 0;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i, ++chars) {
		const unsigned char chr = *i;
		// only one line
		if (chr == '\n') {
			break;
		}
		const FontChar* fontChr = _fontDefPtr->getFontChar(chr);
		if (fontChr == nullptr)
			fontChr = _fontDefPtr->getFontChar(' ');
		lineWidth += fontChr->getWidth();
		if (lineWidth > pixelWidth)
			break;
	}
	return chars;
}

void BitmapFont::update (uint32_t deltaTime)
{
	_time += deltaTime;
}

int BitmapFont::getCharWidth () const
{
	return getTextWidth(" ");
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
			fontChr = _fontDefPtr->getFontChar(' ');
		lineWidth += fontChr->getWidth();
	}
	return std::max(lineWidth, width);
}

int BitmapFont::printMax (const std::string& text, const Color& color, int x, int y, int maxLength, bool rotate) const
{
	if (_fontDefPtr->getHeight() < 8)
		return 0;

	if (text.empty())
		return 0;
	if (color[3] <= 0.0001f)
		return 0;

	const int beginX = x;
	int yShift = 0;

	_frontend->setColor(color);
	const TextureRect sourceRect = _font->getSourceRect();
	for (std::string::const_iterator i = text.begin(); i != text.end(); ++i) {
		const unsigned char chr = *i;
		if (chr == '\n') {
			x = beginX;
			yShift += _fontDefPtr->getHeight();
			continue;
		} else if (chr == '\t') {
			x += 4 * _fontDefPtr->getFontChar(' ')->getWidth();
			continue;
		}
		const FontChar* fontChr = _fontDefPtr->getFontChar(chr);
		if (fontChr == nullptr) {
			x += _fontDefPtr->getFontChar(' ')->getWidth();
			continue;
		}
		if (maxLength <= 0 || x + fontChr->getWidth() - beginX <= maxLength) {
			_font->setRect(sourceRect.x + fontChr->getX(), sourceRect.y + fontChr->getY(), fontChr->getW(), fontChr->getH());
			const int letterAngleMod = x + fontChr->getOX() + y + yShift + _fontDefPtr->getHeight() - fontChr->getOY() + fontChr->getW() + fontChr->getH();
			const int angle = rotate ? RadiansToDegrees(cos(static_cast<double>(letterAngleMod * 100 + _time + _rand) / 100.0) / 6.0) : 0;
			_frontend->renderImage(_font.get(), x + fontChr->getOX(), y + yShift + _fontDefPtr->getHeight() - fontChr->getOY(), fontChr->getW(), fontChr->getH(), angle, color[3]);
		}
		x += fontChr->getWidth();
	}
	_font->setRect(sourceRect.x, sourceRect.y, sourceRect.w, sourceRect.h);
	_frontend->resetColor();
	return yShift;
}

int BitmapFont::print (const std::string& text, const Color& color, int x, int y, bool rotate) const
{
	return printMax(text, color, x, y, -1, rotate);
}
