#include "BGFXFrontend.h"

#ifdef HAVE_BGFX

#include <bgfx.h>

BGFXFrontend::BGFXFrontend (SharedPtr<IConsole> console) :
		SDLFrontend(console)
{
}

BGFXFrontend::~BGFXFrontend ()
{
	bgfx::shutdown();
}

void BGFXFrontend::setGLAttributes ()
{
}

void BGFXFrontend::initRenderer ()
{
	bgfx::init();
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR_BIT | BGFX_CLEAR_DEPTH_BIT, 0x303030ff, 1.0f, 0);
}

void BGFXFrontend::setHints ()
{
}

void BGFXFrontend::enableScissor (int x, int y, int width, int height)
{
}

float BGFXFrontend::getWidthScale () const
{
	return 1.0f;
}

float BGFXFrontend::getHeightScale () const
{
	return 1.0f;
}

void BGFXFrontend::disableScissor ()
{
}

int BGFXFrontend::getCoordinateOffsetX () const
{
	return 0;
}

int BGFXFrontend::getCoordinateOffsetY () const
{
	return 0;
}

void BGFXFrontend::getViewPort (int* x, int *y, int *w, int *h) const
{
}

void BGFXFrontend::bindTexture (Texture* texture, int textureUnit)
{
}

void BGFXFrontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
{
}

void BGFXFrontend::renderRect (int x, int y, int w, int h, const Color& color)
{
}

void BGFXFrontend::renderFilledRect (int x, int y, int w, int h, const Color& fillColor)
{
}

void BGFXFrontend::destroyTexture (TextureData *data)
{
}

bool BGFXFrontend::loadTexture (Texture *texture, const std::string& filename)
{
	return false;
}

void BGFXFrontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
}

void BGFXFrontend::makeScreenshot (const std::string& filename)
{
}

void BGFXFrontend::updateViewport (int x, int y, int width, int height)
{
	bgfx::setViewRect(0, x, y, width, height);
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}

void BGFXFrontend::renderBegin ()
{
}

void BGFXFrontend::renderEnd ()
{
	bgfx::frame();
}

#endif
