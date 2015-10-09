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
#ifdef SDL_VIDEO_OPENGL
	Log::info(LOG_CLIENT, "init opengl renderer");

	_context = SDL_GL_CreateContext(_window);
	ExtGLLoadFunctions();

	GL_checkError();

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

	glClearColor(0, 0, 0, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	GL_checkError();
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	GL_checkError();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_checkError();
	glAlphaFunc(GL_GREATER, 0.1f);
	GL_checkError();
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	GL_checkError();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GL_checkError();
#endif
}

void GL1Frontend::renderBatches ()
{
#ifdef SDL_VIDEO_OPENGL
	SDL_assert_always(_batches[0].texnum != 0);
	bool scissorActive = false;
	for (int i = 0; i <= _currentBatch; ++i) {
		const Batch& b = _batches[i];
		if (b.vertexCount == 0)
			continue;
		if (b.scissor) {
			if (!scissorActive)
				glEnable(GL_SCISSOR_TEST);
			scissorActive = true;
			glScissor(b.scissorRect.x * _rx, b.scissorRect.y * _ry, b.scissorRect.w * _rx, b.scissorRect.h * _ry);
		} else if (scissorActive) {
			glDisable(GL_SCISSOR_TEST);
		}
		if (_currentTexture != b.texnum || _currentNormal != b.normaltexnum) {
			_currentTexture = b.texnum;
			_currentNormal = b.normaltexnum;
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, b.normaltexnum);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, b.texnum);
			GL_checkError();
		}
		uint8_t *start = (uint8_t*)(&(_vertices[b.vertexIndexStart]));
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, x)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, c)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), GL_CALC_OFFSET(start + offsetof(Vertex, u)));
		GL_checkError();
		glDrawArrays(b.type, 0, b.vertexCount);
		GL_checkError();
	}
	if (scissorActive)
		glDisable(GL_SCISSOR_TEST);
	_drawCalls += _currentBatch;
	_currentVertexIndex = 0;

	const SDL_Rect scissorRect = _batches[_currentBatch].scissorRect;
	const bool scissor = _batches[_currentBatch].scissor;
	_currentBatch = 0;
	memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
	_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
	_batches[_currentBatch].scissorRect = scissorRect;
	_batches[_currentBatch].scissor = scissor;
	GL_checkError();
#endif
}

void GL1Frontend::updateViewport (int x, int y, int width, int height)
{
#ifdef SDL_VIDEO_OPENGL
	AbstractGLFrontend::updateViewport(x, y, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const GLdouble _w = static_cast<GLdouble>(_viewPort.w);
	const GLdouble _h = static_cast<GLdouble>(_viewPort.h);
	glOrtho(0.0, _w, _h, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	GL_checkError();
#endif
}
