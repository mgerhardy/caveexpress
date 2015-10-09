#include "AbstractGLFrontend.h"
#include "textures/TextureCoords.h"
#include "common/Log.h"
#include "common/System.h"
#include "common/FileSystem.h"
#include <SDL.h>
#include <SDL_image.h>

AbstractGLFrontend::AbstractGLFrontend (std::shared_ptr<IConsole> console) :
		SDLFrontend(console), _currentTexture(-1), _currentNormal(-1), _rx(1.0f), _ry(1.0f), _renderTargetTexture(0), _alpha(0), _drawCalls(0)
{
	_context = nullptr;
	_currentBatch = 0;
	memset(_batches, 0, sizeof(_batches));
	memset(&_viewPort, 0, sizeof(_viewPort));
}

AbstractGLFrontend::~AbstractGLFrontend ()
{
	if (_context)
		SDL_GL_DeleteContext(_context);
}

void AbstractGLFrontend::setGLAttributes ()
{
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

void AbstractGLFrontend::setHints ()
{
}

RenderTarget* AbstractGLFrontend::renderToTexture (int x, int y, int w, int h)
{
	// render the current batches and then start to write to the fbo texture
	renderBatches();
	static RenderTarget target;
	target.fbo = &_fbo;
	target.fbo->bind(x, y, w, h);
	return &target;
}

void AbstractGLFrontend::bindTargetTexture(RenderTarget* target)
{
}

void AbstractGLFrontend::unbindTargetTexture (RenderTarget* target)
{
}

bool AbstractGLFrontend::renderTarget (RenderTarget* target)
{
	disableRenderTarget(target);
	const TextureRect& r = target->fbo->rect();
	const TextureCoords texCoords(r, r.w, r.h, false, true);
	renderTexture(texCoords, r.x, r.y, r.w, r.h, 0, 1.0, _renderTargetTexture, _alpha);
	return true;
}

bool AbstractGLFrontend::disableRenderTarget (RenderTarget* target)
{
	// render the current batches to the fbo texture - and unbind the fbo
	renderBatches();
	target->fbo->unbind();
	return true;
}

void AbstractGLFrontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
{
	if (texture == nullptr || !texture->isValid())
		return;

	const TextureCoords texCoords(texture);
	getTrimmed(texture, x, y, w, h);
	renderTexture(texCoords, x, y, w, h, angle, alpha, texture->getData()->texnum, texture->getData()->normalnum);
}

/**
 * vertices as tris
 * 1 -- 2
 * 4 \  |
 * |  \ |
 * |    3
 * 6 -- 5
 */
void AbstractGLFrontend::renderTexture(const TextureCoords& texCoords, int x, int y, int w, int h, int16_t angle, float alpha, GLuint texnum, GLuint normaltexnum)
{
	glm::mat4 transform(1.0f);
	transform = glm::translate(transform, glm::vec3(x + w / 2, y + h / 2, 0));
	transform = glm::rotate(transform, (float)DegreesToRadians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
	transform = glm::translate(transform, glm::vec3(-w / 2, -h / 2, 0));
	transform = glm::scale(transform, glm::vec3(w * _rx, h * _ry, 1.0f));
	glm::vec4 vertexTL(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 vertexTR(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 vertexBR(1.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 vertexBL(0.0f, 1.0f, 0.0f, 1.0f);
	vertexTL = transform * vertexTL;
	vertexTR = transform * vertexTR;
	vertexBR = transform * vertexBR;
	vertexBL = transform * vertexBL;

	flushBatch(GL_TRIANGLES, texnum, 6);
	Batch& batch = _batches[_currentBatch];
	batch.normaltexnum = normaltexnum;

	Vertex v(_color);
	v.c.a = alpha * 255.0f;

	v.u = texCoords.texCoords[0];
	v.v = texCoords.texCoords[1];
	v.x = vertexTL.x;
	v.y = vertexTL.y;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[2];
	v.v = texCoords.texCoords[3];
	v.x = vertexTR.x;
	v.y = vertexTR.y;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[4];
	v.v = texCoords.texCoords[5];
	v.x = vertexBR.x;
	v.y = vertexBR.y;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[0];
	v.v = texCoords.texCoords[1];
	v.x = vertexTL.x;
	v.y = vertexTL.y;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[4];
	v.v = texCoords.texCoords[5];
	v.x = vertexBR.x;
	v.y = vertexBR.y;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[6];
	v.v = texCoords.texCoords[7];
	v.x = vertexBL.x;
	v.y = vertexBL.y;
	_vertices[_currentVertexIndex++] = v;
}

void AbstractGLFrontend::flushBatch (int type, GLuint texnum, int vertexAmount)
{
	if (_currentVertexIndex + vertexAmount >= MAXNUMVERTICES)
		renderBatches();
	else if (_currentBatch >= MAX_BATCHES)
		renderBatches();
	SDL_assert_always(vertexAmount < MAXNUMVERTICES);
	const Batch& b = _batches[_currentBatch];
	if (b.type == type && b.texnum == texnum) {
		_batches[_currentBatch].vertexCount += vertexAmount;
		return;
	}
	startNewBatch();
	_batches[_currentBatch].type = type;
	_batches[_currentBatch].texnum = texnum;
	_batches[_currentBatch].vertexCount += vertexAmount;
}

void AbstractGLFrontend::startNewBatch ()
{
	SDL_assert_always(_currentBatch < MAX_BATCHES);
	const Batch& b = _batches[_currentBatch];
	if (_currentVertexIndex - b.vertexIndexStart == 0)
		return;

	if (b.vertexCount == 0)
		return;

	if (_currentVertexIndex + 6 >= MAXNUMVERTICES)
		return;

	++_currentBatch;
	if (_currentBatch >= MAX_BATCHES) {
		Log::debug(LOG_CLIENT, "render the batches because the max batch count was exceeded");
		--_currentBatch;
		renderBatches();
		SDL_assert_always(_currentBatch == 0);
		return;
	}
	memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
	_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
	_batches[_currentBatch].scissorRect = _batches[_currentBatch - 1].scissorRect;
	_batches[_currentBatch].scissor = _batches[_currentBatch - 1].scissor;
}

void AbstractGLFrontend::enableScissor (int x, int y, int width, int height)
{
	const int lowerLeft = _fbo.isBound() ? y : std::max(0, getHeight() - y - height);
	startNewBatch();
	_batches[_currentBatch].scissor = true;
	_batches[_currentBatch].scissorRect.x = x;
	_batches[_currentBatch].scissorRect.y = lowerLeft;
	_batches[_currentBatch].scissorRect.w = width;
	_batches[_currentBatch].scissorRect.h = height;
}

float AbstractGLFrontend::getWidthScale () const
{
	return _rx;
}

float AbstractGLFrontend::getHeightScale () const
{
	return _ry;
}

void AbstractGLFrontend::disableScissor ()
{
	startNewBatch();
	_batches[_currentBatch].scissor = false;
}

int AbstractGLFrontend::getCoordinateOffsetX () const
{
	return -_viewPort.x;
}

int AbstractGLFrontend::getCoordinateOffsetY () const
{
	return -_viewPort.y;
}

void AbstractGLFrontend::getViewPort (int* x, int *y, int *w, int *h) const
{
	if (x != nullptr)
		*x = _viewPort.x;
	if (y != nullptr)
		*y = _viewPort.y;
	if (w != nullptr)
		*w = _viewPort.w;
	if (h != nullptr)
		*h = _viewPort.h;
}

void AbstractGLFrontend::bindTexture (Texture* texture, int textureUnit)
{
	const TexNum texnum = texture->getData()->texnum;
	if (_currentTexture == texnum)
		return;
	_currentTexture = texnum;
	Batch& batch = _batches[_currentBatch];
	batch.texnum = texnum;
	batch.normaltexnum = texture->getData()->normalnum;
	glBindTexture(GL_TEXTURE_2D, _currentTexture);
}

int AbstractGLFrontend::renderFilledPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;
	static const glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(_rx, _ry, 0.0f));

	flushBatch(GL_TRIANGLE_FAN, _white, n);
	Batch& batch = _batches[_currentBatch];
	batch.normaltexnum = _alpha;

	Vertex v(color);

	for (int i = 0; i < n; ++i) {
		v.x = vx[i];
		v.y = vy[i];
		_vertices[_currentVertexIndex++] = v;
	}

	return 0;
}

int AbstractGLFrontend::renderPolygon (int *vx, int *vy, int n, const Color& color)
{
	if (n < 3 || vx == nullptr || vy == nullptr)
		return -1;
	static const glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(_rx, _ry, 0.0f));

	flushBatch(GL_LINES, _white, n + 1);
	Batch& batch = _batches[_currentBatch];
	batch.normaltexnum = _alpha;

	Vertex v(color);

	for (int i = 0; i < n; i += 2) {
		const glm::vec4 pos1 = mat * glm::vec4(vx[i] * _rx, vy[i] * _ry, 0.0f, 1.0f);
		v.x = pos1.x;
		v.y = pos1.y;
		_vertices[_currentVertexIndex++] = v;

		const glm::vec4 pos2 = mat * glm::vec4(vx[i + 1] * _rx, vy[i + 1] * _ry, 0.0f, 1.0f);
		v.x = pos2.x;
		_vertices[_currentVertexIndex++] = v;
		v.y = pos2.y;
	}

	const glm::vec4 start = mat * glm::vec4(vx[0] * _rx, vy[0] * _ry, 0.0f, 1.0f);
	v.x = start.x;
	v.y = start.y;
	_vertices[_currentVertexIndex++] = v;

	return 0;
}

void AbstractGLFrontend::renderFilledRect (int x, int y, int w, int h, const Color& color)
{
	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();

	const glm::mat4 mat = glm::scale(glm::mat4(1.0f), glm::vec3(_rx, _ry, 0.0f));
	const glm::vec4 pos1 = mat * glm::vec4(x, y, 0.0f, 1.0f);
	const glm::vec4 pos2 = mat * glm::vec4(x + w, y + h, 0.0f, 1.0f);

	flushBatch(GL_TRIANGLES, _white, 6);
	Batch& batch = _batches[_currentBatch];
	batch.normaltexnum = _alpha;

	Vertex v(color);

	v.x = pos1.x;
	v.y = pos1.y;
	_vertices[_currentVertexIndex++] = v;

	v.x = pos2.x;
	v.y = pos1.y;
	_vertices[_currentVertexIndex++] = v;

	v.x = pos2.x;
	v.y = pos2.y;
	_vertices[_currentVertexIndex++] = v;

	v.x = pos1.x;
	v.y = pos1.y;
	_vertices[_currentVertexIndex++] = v;

	v.x = pos2.x;
	v.y = pos2.y;
	_vertices[_currentVertexIndex++] = v;

	v.x = pos1.x;
	v.y = pos2.y;
	_vertices[_currentVertexIndex++] = v;
}

void AbstractGLFrontend::renderRect (int x, int y, int w, int h, const Color& color)
{
	renderFilledRect(x, y, w, 1, color);
	renderFilledRect(x, y + h - 1, w, 1, color);
	renderFilledRect(x, y + 1, 1, h - 2, color);
	renderFilledRect(x + w - 1, y + 1, 1, h - 2, color);
}

void AbstractGLFrontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
	flushBatch(GL_LINES, _white, 2);
	Batch& batch = _batches[_currentBatch];
	batch.normaltexnum = _alpha;

	Vertex v(color);
	v.x = x1 * _rx;
	v.y = y1 * _ry;
	_vertices[_currentVertexIndex++] = v;

	v.x = x2 * _rx;
	v.y = y2 * _ry;
	_vertices[_currentVertexIndex++] = v;
}

void AbstractGLFrontend::destroyTexture (TextureData *data)
{
	if (data->texnum != _white && data->texnum != _alpha && data->normalnum != _white && data->normalnum != _alpha) {
		glDeleteTextures(1, &data->texnum);
		glDeleteTextures(1, &data->normalnum);
		GL_checkError();
	}
}

SDL_Surface* AbstractGLFrontend::loadTextureIntoSurface(const std::string& filename) {
	const std::string file = FS.getFileFromURL("pics://" + filename + ".png")->getName();
	SDL_RWops *src = FS.createRWops(file);
	if (src == nullptr) {
		return nullptr;
	}
	SDL_Surface *surface = IMG_Load_RW(src, 1);
	if (!surface) {
		sdlCheckError();
		return nullptr;
	}

	if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
		SDL_PixelFormat *destFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
		if (destFormat == nullptr) {
			SDL_FreeSurface(surface);
			return nullptr;
		}
		SDL_Surface* temp = SDL_ConvertSurface(surface, destFormat, 0);
		SDL_FreeFormat(destFormat);
		if (temp == nullptr) {
			SDL_FreeSurface(surface);
			return nullptr;
		}
		SDL_FreeSurface(surface);
		surface = temp;
	}

	return surface;
}

bool AbstractGLFrontend::loadTexture (Texture *texture, const std::string& filename)
{
	SDL_Surface* textureSurface = loadTextureIntoSurface(filename);
	if (textureSurface == nullptr) {
		Log::error(LOG_CLIENT, "could not load the file: %s", filename.c_str());
		return false;
	}
	SDL_Surface* normalSurface = loadTextureIntoSurface(filename + "_n");
	TextureData* data = new TextureData();
	data->texnum = uploadTexture(static_cast<unsigned char*>(textureSurface->pixels), textureSurface->w, textureSurface->h);
	if (normalSurface) {
		data->normalnum = uploadTexture(static_cast<unsigned char*>(normalSurface->pixels), normalSurface->w, normalSurface->h);
		Log::info(LOG_CLIENT, "load normal map for: %s", filename.c_str());
	} else {
		data->normalnum = _alpha;
	}
	texture->setData(data);
	texture->setRect(0, 0, textureSurface->w, textureSurface->h);
	SDL_FreeSurface(textureSurface);
	if (normalSurface)
		SDL_FreeSurface(normalSurface);
	return data->texnum != 0;
}

TexNum AbstractGLFrontend::uploadTexture (const unsigned char* pixels, int w, int h) const
{
	TexNum texnum;
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_checkError();
	return texnum;
}

void AbstractGLFrontend::makeScreenshot (const std::string& filename)
{
#ifndef EMSCRIPTEN
	const int bytesPerPixel = 3;
	std::unique_ptr<GLubyte> pixels(new GLubyte[bytesPerPixel * _width * _height]);
	int rowPack;
	glGetIntegerv(GL_PACK_ALIGNMENT, &rowPack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	glPixelStorei(GL_PACK_ALIGNMENT, rowPack);

	std::unique_ptr<SDL_Surface> surface(SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			0x000000ff, 0x0000ff00, 0x00ff0000
#else
			0x00ff0000, 0x0000ff00, 0x000000ff
#endif
			, 0));
	if (!surface)
		return;
	const int pitch = _width * bytesPerPixel;
	for (int y = 0; y < _height; ++y)
		memcpy((uint8 *) surface->pixels + surface->pitch * y, (uint8 *) pixels.get() + pitch * (_height - y - 1), pitch);
	const std::string fullFilename = FS.getAbsoluteWritePath() + filename + "-" + dateutil::getDateString() + ".png";
	IMG_SavePNG(surface.get(), fullFilename.c_str());
#endif
}

void AbstractGLFrontend::updateViewport (int x, int y, int width, int height)
{
	const float wantAspect = (float) width / height;
	const float realAspect = (float) _width / _height;

	float scale;
	if (fequals(wantAspect, realAspect)) {
		/* The aspect ratios are the same, just scale appropriately */
		scale = (float) _width / width;
		_viewPort.x = 0;
		_viewPort.w = _width;
		_viewPort.h = _height;
		_viewPort.y = 0;
	} else if (wantAspect > realAspect) {
		/* We want a wider aspect ratio than is available - letterbox it */
		scale = (float) _width / width;
		_viewPort.x = 0;
		_viewPort.w = _width;
		_viewPort.h = (int) SDL_ceil(height * scale);
		_viewPort.y = (_height - _viewPort.h) / 2;
	} else {
		/* We want a narrower aspect ratio than is available - use side-bars */
		scale = (float) _height / height;
		_viewPort.y = 0;
		_viewPort.h = _height;
		_viewPort.w = (int) SDL_ceil(width * scale);
		_viewPort.x = (_width - _viewPort.w) / 2;
	}

	_rx = scale;
	_ry = scale;

	_fbo.destroy();
	_fbo.bind();
	_renderTargetTexture = _fbo.createTexture(GL_COLOR_ATTACHMENT0, _viewPort.w, _viewPort.h);
	_fbo.drawBuffer();
	SDL_assert_always(_fbo.isSuccessful());
	_fbo.unbind();

	glViewport(_viewPort.x, _viewPort.y, _viewPort.w, _viewPort.h);
	const GLdouble _w = static_cast<GLdouble>(_viewPort.w);
	const GLdouble _h = static_cast<GLdouble>(_viewPort.h);
	_projectionMatrix = glm::ortho(0.0, _w, _h, 0.0, -1.0, 1.0);
	GL_checkError();
}

void AbstractGLFrontend::renderBegin ()
{
	SDL_GL_MakeCurrent(_window, _context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	_currentBatch = 0;
	_drawCalls = 0;
}

void AbstractGLFrontend::renderEnd ()
{
	renderBatches();
#ifdef DEBUG
	Log::debug(LOG_CLIENT, "%i drawcalls", _drawCalls);
#endif
	SDL_GL_SwapWindow(_window);
	GL_checkError();
}
