#pragma once

#include "SDLFrontend.h"
#include "engine/common/GLShared.h"

typedef unsigned int TexNum;

#define MAX_GL_TEXUNITS 4
#define MAX_COLOR 4

class GLFrontend: public SDLFrontend {
protected:
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GLContext _context;
#endif

	struct TexUnit {
		TexUnit() :
				active(false), textureUnit(0), currentTexture(0) {
		}
		bool active;
		int textureUnit;
		TexNum currentTexture;

		inline bool operator< (const TexUnit& other) const
		{
			return textureUnit < other.textureUnit;
		}

		inline bool operator== (const TexUnit& other) const
		{
			return textureUnit == other.textureUnit;
		}

		inline bool operator!= (const TexUnit& other) const
		{
			return textureUnit != other.textureUnit;
		}
	};

	TexUnit _texUnits[MAX_GL_TEXUNITS];
	int _maxTextureUnits;
	TexUnit* _currentTextureUnit;

	// aspect ratio
	float _rx;
	float _ry;

	SDL_Rect _viewPort;

	GLfloat _colorArray[MAX_COLOR * 4];

	void selectTextureUnit (TexUnit &textureUnit);
	void enableTextureUnit (TexUnit &texunit, bool enable);
	bool invalidTexUnit (int textureUnit) const;
	bool checkExtension (const char *extension) const;
	uintptr_t getProcAddress (const char *functionName) const;
	void setColorPointer (const Color& color, int amount);

public:
	GLFrontend (SharedPtr<IConsole> console);
	virtual ~GLFrontend ();

	void renderBegin () override;
	void renderEnd () override;

	void makeScreenshot (const std::string& filename) override;
	int getCoordinateOffsetX () const override;
	int getCoordinateOffsetY () const override;
	void getViewPort (int* x, int *y, int *w, int *h) const override;
	bool loadTexture (Texture *texture, const std::string& filename) override;
	void destroyTexture (void *data) override;
	void renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha = 1.0f) override;
	void bindTexture (Texture* texture, int textureUnit) override;
	void renderRect (int x, int y, int w, int h, const Color& color) override;
	void renderFilledRect (int x, int y, int w, int h, const Color& fillColor) override;
	void renderLine (int x1, int y1, int x2, int y2, const Color& color) override;
	void initRenderer () override;
	void setGLAttributes () override;
	void setHints () override;
	float getWidthScale () const override;
	float getHeightScale () const override;
	void enableScissor (int x, int y, int width, int height) override;
	void disableScissor () override;
	void updateViewport (int x, int y, int width, int height) override;
};
