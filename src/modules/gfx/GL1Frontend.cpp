#include "GL1Frontend.h"
#include "ui/UI.h"
#include "client/ClientConsole.h"
#include "textures/TextureCoords.h"
#include "common/Log.h"
#include "common/DateUtil.h"
#include "common/FileSystem.h"
#include "common/ConfigManager.h"
#include <SDL.h>

GL1Frontend::GL1Frontend (std::shared_ptr<IConsole> console) :
		AbstractGLFrontend(console)
{
}

GL1Frontend::~GL1Frontend ()
{
}

void GL1Frontend::initRenderer ()
{
	AbstractGLFrontend::initRenderer();
	Log::info(LOG_GFX, "init opengl renderer");

#ifdef SDL_VIDEO_OPENGL
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(_identity));

	GL_checkError();
#endif
}

void GL1Frontend::renderBatches ()
{
#ifdef SDL_VIDEO_OPENGL
	uint8_t *start = (uint8_t*)_vertices;
	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, x)));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, c)));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, u)));
#endif
	renderBatchBuffers();
}

void GL1Frontend::updateViewport (int x, int y, int width, int height)
{
	AbstractGLFrontend::updateViewport(x, y, width, height);
#ifdef SDL_VIDEO_OPENGL
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(_projectionMatrix));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(_identity));
	GL_checkError();
#endif
}
