#include "BitmapFont.h"
#include "common/IFrontend.h"
#include "UI.h"
#include "common/Log.h"
#include "common/System.h"

BitmapFont::BitmapFont(const FontDefPtr& fontDefPtr, IFrontend *frontend) :
		_frontend(frontend),_fontDefPtr(fontDefPtr), _time(0U) {
	_font = UI::get().loadTexture(_fontDefPtr->textureName);
	_rand = randBetween(0, 10000);
	if (!_font->isValid()) {
		Log::error(LOG_UI, "invalid font definition with texture %s", _fontDefPtr->textureName.c_str());
		SDL_assert_always(_font->isValid());
	}
	_fontDefPtr->updateChars(_font->getTrim().untrimmedWidth, _font->getTrim().untrimmedHeight);
}

BitmapFont::~BitmapFont (void)
{
	Log::info(LOG_UI, "free the bitmap font %s", _fontDefPtr->id.c_str());
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
		if (fontChr == nullptr)
			continue;
		lineWidth += fontChr->getWidth();
	}
	return std::max(lineWidth, width);
}

int BitmapFont::printMax (const std::string& text, const Color& color, int x, int y, int maxLength, bool rotate) const
{
	SDL_assert_always(_fontDefPtr);
	SDL_assert_always(_frontend);
	SDL_assert_always(_font->isValid());

	if (_fontDefPtr->getHeight() < 5)
		return 0;

	if (text.empty())
		return 0;
	if (color[3] <= 0.0001f)
		return 0;

	const int beginX = x;
	int yShift = 0;

	// would be too expensive
	if (_frontend->isSoftwareRenderer()) {
		rotate = false;
	}

	_frontend->setColor(color);
	const int fontHeight = _fontDefPtr->getHeight();
	const TextureRect sourceRect = _font->getSourceRect();
	const FontChar* space = _fontDefPtr->getFontChar(' ');
	SDL_assert_always(space);
	for (std::string::const_iterator i = text.begin(); i != text.end(); ++i) {
		const unsigned char chr = *i;
		if (chr == '\n') {
			x = beginX;
			yShift += fontHeight;
			continue;
		} else if (chr == '\t') {
			x += 4 * space->getWidth();
			continue;
		}
		const FontChar* fontChr = _fontDefPtr->getFontChar(chr);
		if (fontChr == nullptr) {
			x += space->getWidth();
			continue;
		}
		if (maxLength <= 0 || x + fontChr->getWidth() - beginX <= maxLength) {
			const int fcx = fontChr->getX();
			const int fcy = fontChr->getY();
			const int fcw = fontChr->getW();
			const int fch = fontChr->getH();
			_font->setRect(sourceRect.x + fcx, sourceRect.y + fcy, fcw, fch);
			int angle;
			if (rotate && _time > 0u) {
				const int letterAngleMod = x + fontChr->getOX() + y + yShift + fontHeight - fontChr->getOY() + fcw + fch;
				angle = RadiansToDegrees(cos(static_cast<double>(letterAngleMod * 100 + _time + _rand) / 100.0) / 6.0);
			} else {
				angle = 0;
			}
			_frontend->renderImage(_font.get(), x + fontChr->getOX(), y + yShift + fontHeight - fontChr->getOY(), fcw, fch, angle, color[3]);
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
