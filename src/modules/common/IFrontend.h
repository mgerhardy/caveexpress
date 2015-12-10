#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "common/Math.h"
#include "common/EventHandler.h"
#include <memory>
#include <SDL_platform.h>

// forward declarations
class ServiceProvider;
class Texture;
class BitmapFont;
typedef std::shared_ptr<BitmapFont> BitmapFontPtr;

struct RenderTarget;
struct TextureData;

#define TINY_FONT "font-8"
#define SMALL_FONT "font-10"
#define MEDIUM_FONT "font-12"
#define HUGE_FONT "font-24"
#define LARGE_FONT "font-48"

#define DEFAULT_FONT MEDIUM_FONT

// class that represents a frontend for the server. This might be a console based frontend for
// dedicated servers, as well as a rendering client for listening servers or game clients.
class IFrontend {
protected:
	int _width;
	int _height;

public:
	IFrontend() :
			_width(0), _height(0)
	{
	}

	virtual ~IFrontend ()
	{
	}

	inline int getHeight() const
	{
		return _height;
	}

	inline int getWidth() const
	{
		return _width;
	}

	virtual float getWidthScale () const
	{
		return 1.0f;
	}

	virtual float getHeightScale () const
	{
		return 1.0f;
	}

	virtual void getViewPort (int* x, int *y, int *w, int *h) const
	{
		if (x != nullptr)
			*x = 0;
		if (y != nullptr)
			*y = 0;
		if (w != nullptr)
			*w = getWidth();
		if (h != nullptr)
			*h = getHeight();
	}

	virtual int getCoordinateOffsetX () const
	{
		return 0;
	}

	virtual int getCoordinateOffsetY () const
	{
		return 0;
	}

	virtual void makeScreenshot (const std::string& filename) {}
	virtual bool isConsoleActive () const { return false; }
	virtual void setFullscreen (bool fullscreen) = 0;
	virtual void toggleGrabMouse () {}
	virtual bool isFullscreen () = 0;
	virtual void renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha) = 0;
	virtual void renderBatches () {}
	virtual void setCursorPosition (int x, int y) = 0;
	virtual void showCursor (bool show) = 0;
	virtual bool loadTexture (Texture *texture, const std::string& filename) = 0;
	virtual void bindTexture (Texture* texture, int textureUnit) = 0;
	virtual RenderTarget* renderToTexture (int x, int y, int w, int h) { return nullptr; }
	virtual bool renderTarget (RenderTarget* target) { return false; }
	virtual bool disableRenderTarget (RenderTarget* target) { return false; }
	virtual void bindTargetTexture (RenderTarget* target) {}
	virtual void unbindTargetTexture (RenderTarget* target) {}
	// @param w the width of the rect to fill <= 0 to use the full screen width
	// @param h the height of the rect to fill <= 0 to use the full screen height
	virtual void renderRect (int x, int y, int w, int h, const Color& color) = 0;
	virtual bool renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& waterLineColor) {
		renderLine(x, y - 1, x + w, y - 1, waterLineColor);
		renderFilledRect(x, y, w, h, fillColor);
		return true;
	}
	// @param w the width of the rect to fill <= 0 to use the full screen width
	// @param h the height of the rect to fill <= 0 to use the full screen height
	virtual void renderFilledRect (int x, int y, int w, int h, const Color& fillColor) = 0;
	virtual int renderFilledPolygon (int *vx, int *vy, int n, const Color& color) = 0;
	virtual int renderPolygon (int *vx, int *vy, int n, const Color& color) = 0;
	virtual void renderLine (int x1, int y1, int x2, int y2, const Color& color) = 0;
	virtual void renderLineWithTexture (int x1, int y1, int x2, int y2, Texture* texture) = 0;
	virtual void updateViewport (int x, int y, int width, int height) = 0;
	virtual void enableScissor (int x, int y, int width, int height) = 0;
	virtual void disableScissor () = 0;
	virtual void destroyTexture (TextureData *data) = 0;
	virtual void minimize () = 0;
	virtual bool rumble (float strength, int lengthMillis) { return false; }
	virtual bool hasJoystick () const { return false; }
	virtual bool hasMouse () const {
#if defined(__WINDOWS__) or defined(__LINUX__) or defined(__MACOSX__) or defined(__PNACL__) or defined(__EMSCRIPTEN__)
		return true;
#else
		return false;
#endif
	}

	virtual bool setFrameCallback (int interval, void (*callback)(void*), void *callbackParam) { return false; }

	virtual void resetColor () {}
	virtual void setColor (const Color& rgba) {}

	// rendering methods
	virtual void render () = 0;

	// called each frame to update the frontend
	virtual void update (uint32_t deltaTime) = 0;

	// called to connect to a local running server
	virtual void connect () = 0;

	// called as early as possible to e.g. render a loading screen
	virtual void onInit () = 0;

	// called once when we enter the mainloop
	virtual void onStart () = 0;

	virtual void shutdown () = 0;

	// called whenever a new map was loaded
	virtual void onMapLoaded () = 0;

	// if this returns true, the input is completly handled in the frontend and the backend
	// will e.g. not resolve any keybindings
	virtual bool handlesInput () const = 0;

	// initialize the frontend
	virtual int init (int width, int height, bool fullscreen, EventHandler &eventHandler) = 0;

	virtual void initUI (ServiceProvider& serviceProvider) = 0;
};
