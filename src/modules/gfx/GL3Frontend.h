#pragma once

#include "SDLFrontend.h"
#include "shaders/Shader.h"
#include "FrameBuffer.h"

class TextureCoords;

class GL3Frontend: public SDLFrontend {
protected:
	SDL_GLContext _context;

	TexNum _currentTexture;
	TexNum _currentNormal;

	// aspect ratio
	float _rx;
	float _ry;
	glm::mat4 _projectionMatrix;
	SDL_Rect _viewPort;
	GLuint _vao;
	GLuint _vbo;
	FrameBuffer _fbo;
	GLuint _renderTargetTexture;
	TexNum _white;
	TexNum _alpha;
	TexNum _waterNoise;
	Shader _shader;
	Shader _waterShader;
	int _drawCalls;

	bool checkExtension (const char *extension) const;
	uintptr_t getProcAddress (const char *functionName) const;
	TexNum uploadTexture(const unsigned char* pixels, int w, int h) const;
	void flushBatch (int type, GLuint texnum, int vertexAmount);
	void startNewBatch ();
	void renderTexture(const TextureCoords& texCoords, int x, int y, int w, int h, int16_t angle, float alpha, GLuint texnum, GLuint normaltexnum);
	SDL_Surface* loadTextureIntoSurface(const std::string& file);
	void renderBatchesWithShader (Shader& shader);

public:
	explicit GL3Frontend (std::shared_ptr<IConsole> console);
	virtual ~GL3Frontend ();

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
	bool renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& waterLineColor) override;
	bool renderTarget (RenderTarget* target) override;
	bool disableRenderTarget (RenderTarget* target) override;
	void bindTargetTexture (RenderTarget* target) override;
	void unbindTargetTexture (RenderTarget* target) override;
	void renderRect (int x, int y, int w, int h, const Color& color) override;
	void renderFilledRect (int x, int y, int w, int h, const Color& fillColor) override;
	void renderLine (int x1, int y1, int x2, int y2, const Color& color) override;
	void initRenderer () override;
	void setGLAttributes () override;
	void setHints () override;
	void renderBatches () override;
	float getWidthScale () const override;
	float getHeightScale () const override;
	void enableScissor (int x, int y, int width, int height) override;
	void disableScissor () override;
	void updateViewport (int x, int y, int width, int height) override;
};
