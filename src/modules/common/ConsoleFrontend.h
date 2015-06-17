#pragma once

#include "common/IFrontend.h"
#include "common/TextConsole.h"

class EventHandler;

class ConsoleFrontend: public IFrontend {
private:
	EventHandler *_eventHandler;
	TextConsole& _console;

public:
	explicit ConsoleFrontend (TextConsole& console);
	virtual ~ConsoleFrontend ();

	// IFrontend
	void setFullscreen (bool fullscreen) override {}
	bool isFullscreen () override { return true; }
	void renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha) override {}
	void setCursorPosition (int x, int y) override {}
	void showCursor (bool show) override {}
	bool loadTexture (Texture* texture, const std::string& filename) override { return false; }
	void bindTexture (Texture* texture, int textureUnit) override {}
	void renderRect (int x, int y, int w, int h, const Color& color) override {}
	void renderFilledRect (int x, int y, int w, int h, const Color& color) override {}
	void renderLine (int x1, int y1, int x2, int y2, const Color& color) override {}
	void renderLineWithTexture (int x1, int y1, int x2, int y2, Texture* texture) override {}
	int renderFilledPolygon (int *vx, int *vy, int n, const Color& color) override { return -1; }
	int renderPolygon (int *vx, int *vy, int n, const Color& color) override { return -1; }
	void updateViewport (int x, int y, int width, int height) override {}
	void enableScissor (int x, int y, int width, int height) override {}
	void destroyTexture (TextureData *data) override {}
	void disableScissor () override {}
	void minimize () override {}
	void render () override;
	void connect () override;
	void update (uint32_t deltaTime) override;
	void onStart() override {}
	void onMapLoaded () override;
	void onInit () override {}
	void shutdown () override;
	bool handlesInput () const override;
	int init (int width, int height, bool fullscreen, EventHandler &eventHandler) override;
	void initUI (ServiceProvider& serviceProvider) override;
};
