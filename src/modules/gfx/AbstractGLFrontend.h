#pragma once

#include "SDLFrontend.h"
#include "FrameBuffer.h"
#include "GLFunc.h"
#include "GLShared.h"

struct Vertex {
	Vertex() : x(0.0f), y(0.0f), u(0.0f), v(0.0f) {
		c.c = 1u;
	}

	Vertex(const Color& color) : x(0.0f), y(0.0f), u(0.0f), v(0.0f) {
		c.r = color[0] * 255.0f;
		c.g = color[1] * 255.0f;
		c.b = color[2] * 255.0f;
		c.a = color[3] * 255.0f;
	}

	float x, y;
	float u, v;
	union {
		struct {
			uint8_t r, g, b, a;
		};
		uint32_t c;
	} c;
};

struct TextureData {
	GLuint texnum;
	GLuint normalnum;
};

struct RenderTarget {
	FrameBuffer* fbo;
};

#define MAXNUMVERTICES 0x10000
#define MAX_BATCHES 128

struct Batch {
	TexNum texnum;
	TexNum normaltexnum;
	Vertex vertices[MAXNUMVERTICES];
	int type;
	int vertexIndexStart;
	int vertexCount;
	bool scissor;
	SDL_Rect scissorRect;
};

class TextureCoords;

class AbstractGLFrontend: public SDLFrontend {
protected:
	SDL_GLContext _context;

	TexNum _currentTexture;
	TexNum _currentNormal;

	// aspect ratio
	float _rx;
	float _ry;
	glm::mat4 _projectionMatrix;
	glm::mat4 _identity;
	SDL_Rect _viewPort;
	FrameBuffer _fbo;
	TexNum _renderTargetTexture;
	TexNum _white;
	TexNum _alpha;

	Vertex _vertices[MAXNUMVERTICES];
	int _currentVertexIndex;
	Batch _batches[MAX_BATCHES];
	int _currentBatch;

	TexNum uploadTexture(const unsigned char* pixels, int w, int h) const;
	void flushBatch (int type, GLuint texnum, int vertexAmount);
	void startNewBatch ();
	void renderTexture(const TextureCoords& texCoords, int x, int y, int w, int h, int16_t angle, float alpha, GLuint texnum, GLuint normaltexnum);
	SDL_Surface* loadTextureIntoSurface(const std::string& file);
	void renderBatchBuffers();

public:
	explicit AbstractGLFrontend (std::shared_ptr<IConsole> console);
	virtual ~AbstractGLFrontend ();

	void renderBegin () override;
	void renderEnd () override;

	void makeScreenshot (const std::string& filename) override;
	int getCoordinateOffsetX () const override;
	int getCoordinateOffsetY () const override;
	void getViewPort (int* x, int *y, int *w, int *h) const override;
	bool loadTexture (Texture *texture, const std::string& filename) override;
	void destroyTexture (TextureData *data) override;
	void renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha = 1.0f) override;
	void bindTexture (Texture* texture, int textureUnit) override;
	RenderTarget* renderToTexture (int x, int y, int w, int h) override;
	bool renderTarget (RenderTarget* target) override;
	bool disableRenderTarget (RenderTarget* target) override;
	void bindTargetTexture (RenderTarget* target) override;
	void unbindTargetTexture (RenderTarget* target) override;
	void renderRect (int x, int y, int w, int h, const Color& color) override;
	void renderFilledRect (int x, int y, int w, int h, const Color& fillColor) override;
	int renderFilledPolygon (int *vx, int *vy, int n, const Color& color) override;
	int renderPolygon (int *vx, int *vy, int n, const Color& color) override;
	void renderLine (int x1, int y1, int x2, int y2, const Color& color) override;
	void renderLineWithTexture (int x1, int y1, int x2, int y2, Texture* texture) override;
	void setGLAttributes () override;
	void setHints () override;
	float getWidthScale () const override;
	float getHeightScale () const override;
	void enableScissor (int x, int y, int width, int height) override;
	void disableScissor () override;
	virtual void updateViewport (int x, int y, int width, int height) override;
	virtual void initRenderer() override;
};
