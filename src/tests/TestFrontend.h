#pragma once

#include "engine/common/IFrontend.h"
#include "engine/common/TextConsole.h"
#include "engine/client/textures/Texture.h"

// forward decl
class EventHandler;
class ServiceProvider;

class TestFrontend: public IFrontend {
public:
	TestFrontend(int width = 1024, int height = 768) {
		_height = height;
		_width = width;
	}
	virtual ~TestFrontend() {
	}

	// IFrontend
	void setFullscreen(bool fullscreen) override {
	}

	bool isFullscreen() override {
		return true;
	}

	void setCursorPosition(int x, int y) override {
	}

	void showCursor(bool show) override {
	}

	void renderImage(Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha) override {
	}

	bool loadTexture(Texture* texture, const std::string& filename) override {
		texture->setData(reinterpret_cast<void*>(1));
		texture->setRect(0, 0, 100, 100);
		return true;
	}

	void bindTexture(Texture* texture, int textureUnit) override {
	}

	void renderRect(int x, int y, int w, int h, const Color& color) override {
	}

	void renderFilledRect(int x, int y, int w, int h, const Color& color) override {
	}

	void renderLine(int x1, int y1, int x2, int y2, const Color& color) override {
	}

	void renderLineWithTexture(int x1, int y1, int x2, int y2, Texture* texture) override {
	}

	int renderFilledPolygon(int *vx, int *vy, int n, const Color& color) override {
		return -1;
	}

	int renderPolygon(int *vx, int *vy, int n, const Color& color) override {
		return -1;
	}

	void updateViewport(int x, int y, int width, int height) override {
	}

	void enableScissor(int x, int y, int width, int height) override {
	}

	void destroyTexture(void *data) override {
	}

	void disableScissor() override {
	}

	void render() override {
	}

	void connect() override {
	}

	void update(uint32_t deltaTime) override {
	}

	void onStart() override {
	}

	void onInit() override {
	}

	void minimize() override {
	}

	void shutdown() override {
	}

	void onMapLoaded() override {
	}

	bool handlesInput() const override {
		return false;
	}

	int init(int width, int height, bool fullscreen, EventHandler &eventHandler) override {
		return 1;
	}

	void initUI(ServiceProvider& serviceProvider) override {
	}
};
