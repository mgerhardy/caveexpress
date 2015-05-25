#include "GL3Frontend.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ClientConsole.h"
#include "engine/client/textures/TextureCoords.h"
#include "engine/common/Logger.h"
#include "engine/common/DateUtil.h"
#include "engine/common/FileSystem.h"
#include "engine/common/ConfigManager.h"
#include <SDL_image.h>

struct Vertex {
	float x, y;
	float u, v;
	union {
		struct {
			uint8_t r, g, b, a;
		};
		uint32_t c;
	} c;
};

#define MAXNUMVERTICES 0x10000
#define MAX_BATCHES 128

struct Batch {
	TexNum texnum;
	Vertex vertices[MAXNUMVERTICES];
	int type;
	int vertexIndexStart;
	int vertexCount;
	bool scissor;
	SDL_Rect scissorRect;
	glm::vec2 translation;
	float angle;
};

static Vertex _vertices[MAXNUMVERTICES];
static int _currentVertexIndex;
static Batch _batches[MAX_BATCHES];
static int _currentBatch;

inline TexNum getTexNum (void *textureData)
{
	const intptr_t texnum = reinterpret_cast<intptr_t>(textureData);
	return texnum;
}

GL3Frontend::GL3Frontend (SharedPtr<IConsole> console) :
		SDLFrontend(console), _currentTexture(-1), _rx(1.0f), _ry(1.0f), _vao(0u), _vbo(0u), _white(0), _drawCalls(0)
{
	_context = nullptr;
	_currentBatch = 0;
	memset(_batches, 0, sizeof(_batches));
	memset(&_viewPort, 0, sizeof(_viewPort));
}

GL3Frontend::~GL3Frontend ()
{
	if (_context)
		SDL_GL_DeleteContext(_context);
}

void GL3Frontend::setGLAttributes ()
{
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

void GL3Frontend::setHints ()
{
}

void GL3Frontend::renderBatches ()
{
	_shader.activate();
	if (_shader.hasUniform("u_projection"))
		_shader.setUniformMatrix("u_projection", _projectionMatrix, false);
	if (_shader.hasUniform("u_time"))
		_shader.setUniformi("u_time", SDL_GetTicks());
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _currentVertexIndex, _vertices, GL_DYNAMIC_DRAW);
	bool scissorActive = false;
	for (int i = 0; i < _currentBatch; ++i) {
		Batch& b = _batches[i];
		if (b.scissor) {
			scissorActive = true;
			glScissor(b.scissorRect.x * _rx, b.scissorRect.y * _ry, b.scissorRect.w * _rx, b.scissorRect.h * _ry);
			glEnable(GL_SCISSOR_TEST);
		} else if (scissorActive) {
			glDisable(GL_SCISSOR_TEST);
		}
		if (_currentTexture != b.texnum) {
			_currentTexture = b.texnum;
			glBindTexture(GL_TEXTURE_2D, _currentTexture);
		}
		const glm::mat4& translate = glm::translate(glm::mat4(1.0f), glm::vec3(b.translation, 0.0f));
		const glm::mat4& model = glm::rotate(translate, b.angle, glm::vec3(0.0, 0.0, 1.0));
		_shader.setUniformMatrix("u_model", model);

		glDrawArrays(b.type, b.vertexIndexStart, b.vertexCount);
	}
	if (scissorActive)
		glDisable(GL_SCISSOR_TEST);
	_drawCalls += _currentBatch;
	_currentVertexIndex = 0;
	glBindVertexArray(0);
	_currentBatch = 0;
	_shader.deactivate();
	GL_checkError();
}

void GL3Frontend::flushBatch (int type)
{
//	if (b.type == type)
//		return;
	startNewBatch();
	_batches[_currentBatch].type = type;
}

void GL3Frontend::startNewBatch ()
{
	const Batch& b = _batches[_currentBatch];
	if (_currentVertexIndex - b.vertexIndexStart == 0)
		return;

	++_currentBatch;
	if (_currentBatch >= MAX_BATCHES) {
		debug(LOG_CLIENT, "render the batches because the max batch count was exceeded");
		renderBatches();
		memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
		_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
		return;
	}
	memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
	_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
}

void GL3Frontend::enableScissor (int x, int y, int width, int height)
{
	const int lowerLeft = std::max(0, getHeight() - y - height);
	startNewBatch();
	_batches[_currentBatch].scissorRect.x = x;
	_batches[_currentBatch].scissorRect.y = lowerLeft;
	_batches[_currentBatch].scissorRect.w = width;
	_batches[_currentBatch].scissorRect.h = height;
}

float GL3Frontend::getWidthScale () const
{
	return _rx;
}

float GL3Frontend::getHeightScale () const
{
	return _ry;
}

void GL3Frontend::disableScissor ()
{
	startNewBatch();
	_batches[_currentBatch].scissorRect.x = -1;
	_batches[_currentBatch].scissorRect.y = -1;
	_batches[_currentBatch].scissorRect.w = -1;
	_batches[_currentBatch].scissorRect.h = -1;
}

void GL3Frontend::initRenderer ()
{
	info(LOG_CLIENT, "init opengl renderer");
	_context = SDL_GL_CreateContext(_window);
	ExtGLLoadFunctions();

	glClearColor(0, 0, 0, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	GL_checkError();
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	GL_checkError();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_checkError();
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	GL_checkError();

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	unsigned char white[16];
	memset(white, 0xff, sizeof(white));
	_white = uploadTexture(white, 2, 2);

	memset(_batches, 0, sizeof(_batches));
	_currentVertexIndex = 0;

	if (!_shader.loadProgram("main"))
		error(LOG_CLIENT, "Failed to load the main shader");
	_shader.activate();
	if (_shader.hasUniform("u_texture"))
		_shader.setUniformi("u_texture", 0);
	_shader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, x)));
	_shader.enableVertexAttributeArray("a_pos");
	_shader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, u)));
	_shader.enableVertexAttributeArray("a_texcoord");
	_shader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, c)));
	_shader.enableVertexAttributeArray("a_color");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	_shader.deactivate();
}

int GL3Frontend::getCoordinateOffsetX () const
{
	return -_viewPort.x;
}

int GL3Frontend::getCoordinateOffsetY () const
{
	return -_viewPort.y;
}

void GL3Frontend::getViewPort (int* x, int *y, int *w, int *h) const
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

void GL3Frontend::bindTexture (Texture* texture, int textureUnit)
{
	const TexNum texnum = getTexNum(texture->getData());
	if (_currentTexture == texnum)
		return;
	_currentTexture = texnum;
	Batch& batch = _batches[_currentBatch];
	batch.texnum = texnum;
	glBindTexture(GL_TEXTURE_2D, _currentTexture);
}

void GL3Frontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
{
	if (!texture->isValid())
		return;

	const TextureCoords texCoords(texture);
	getTrimmed(texture, x, y, w, h);
	const float x1 = x * _rx;
	const float y1 = y * _ry;
	const float nw = w * _rx;
	const float nh = h * _ry;
	const float centerx = nw / 2.0f;
	const float centery = nh / 2.0f;
	const float minx = -centerx;
	const float maxx = centerx;
	const float miny = -centery;
	const float maxy = centery;

	const TexNum texnum = getTexNum(texture->getData());
	flushBatch(GL_TRIANGLES);
	Batch& batch = _batches[_currentBatch];
	batch.texnum = texnum;
	batch.angle = DegreesToRadians(angle);
	batch.translation.x = x1 + centerx;
	batch.translation.y = y1 + centery;
	batch.vertexCount += 6;

	Vertex v;
	v.c.r = _color[0] * 255.0f;
	v.c.g = _color[1] * 255.0f;
	v.c.b = _color[2] * 255.0f;
	v.c.a = alpha * 255.0f;

	v.u = texCoords.texCoords[0];
	v.v = texCoords.texCoords[1];
	v.x = minx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[2];
	v.v = texCoords.texCoords[3];
	v.x = maxx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[4];
	v.v = texCoords.texCoords[5];
	v.x = maxx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[0];
	v.v = texCoords.texCoords[1];
	v.x = minx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[4];
	v.v = texCoords.texCoords[5];
	v.x = maxx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;

	v.u = texCoords.texCoords[6];
	v.v = texCoords.texCoords[7];
	v.x = minx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;
}

void GL3Frontend::renderFilledRect (int x, int y, int w, int h, const Color& color)
{
	if (w <= 0)
		w = getWidth();
	if (h <= 0)
		h = getHeight();

	const float nx = x * _rx;
	const float ny = y * _ry;
	const float nw = w * _rx;
	const float nh = h * _ry;
	const float minx = nx;
	const float maxx = nx + nw;
	const float miny = ny;
	const float maxy = ny + nh;

	flushBatch(GL_TRIANGLES);
	Batch& batch = _batches[_currentBatch];
	batch.texnum = _white;
	batch.angle = 0.0f;
	batch.scissor = false;
	batch.scissorRect = {0, 0, 0, 0};
	batch.vertexCount += 6;

	Vertex v;
	v.u = v.v = 0.0f;
	v.c.r = color[0] * 255.0f;
	v.c.g = color[1] * 255.0f;
	v.c.b = color[2] * 255.0f;
	v.c.a = color[3] * 255.0f;

	v.x = minx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.x = maxx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.x = minx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;

	v.x = maxx;
	v.y = miny;
	_vertices[_currentVertexIndex++] = v;

	v.x = minx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;

	v.x = maxx;
	v.y = maxy;
	_vertices[_currentVertexIndex++] = v;
}

void GL3Frontend::renderRect (int x, int y, int w, int h, const Color& color)
{
	renderFilledRect(x, y, w, 1, color);
	renderFilledRect(x, y + h - 1, w, 1, color);
	renderFilledRect(x, y + 1, 1, h - 2, color);
	renderFilledRect(x + w - 1, y + 1, 1, h - 2, color);
}

void GL3Frontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
	flushBatch(GL_LINES);
	Batch& batch = _batches[_currentBatch];
	batch.texnum = _white;
	batch.angle = 0.0f;
	batch.scissor = false;
	batch.scissorRect = {0, 0, 0, 0};
	batch.vertexCount += 2;

	Vertex v;
	v.x = x1 * _rx;
	v.y = y1 * _ry;
	v.u = v.v = 0.0f;
	v.c.r = color[0] * 255.0f;
	v.c.g = color[1] * 255.0f;
	v.c.b = color[2] * 255.0f;
	v.c.a = color[3] * 255.0f;
	_vertices[_currentVertexIndex++] = v;

	v.x = x2 * _rx;
	v.y = y2 * _ry;
	_vertices[_currentVertexIndex++] = v;
}

void GL3Frontend::destroyTexture (void *data)
{
	const TexNum texnum = getTexNum(data);
	glDeleteTextures(1, &texnum);
	GL_checkError();
}

bool GL3Frontend::loadTexture (Texture *texture, const std::string& filename)
{
	const std::string file = FS.getFile(FS.getPicsDir() + filename + ".png")->getName();
	SDL_RWops *src = FS.createRWops(file);
	if (src == nullptr) {
		error(LOG_CLIENT, "could not load the file: " + file);
		return false;
	}
	SDL_Surface *surface = IMG_Load_RW(src, 1);
	if (!surface) {
		sdlCheckError();
		error(LOG_CLIENT, "could not load the texture: " + file);
		return false;
	}

	if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
		SDL_PixelFormat *destFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
		if (destFormat == nullptr) {
			SDL_FreeSurface(surface);
			return false;
		}
		SDL_Surface* temp = SDL_ConvertSurface(surface, destFormat, 0);
		SDL_FreeFormat(destFormat);
		if (temp == nullptr) {
			SDL_FreeSurface(surface);
			return false;
		}
		SDL_FreeSurface(surface);
		surface = temp;
	}

	TexNum texnum = uploadTexture(static_cast<unsigned char*>(surface->pixels), surface->w, surface->h);
	texture->setData(reinterpret_cast<void*>(texnum));
	texture->setRect(0, 0, surface->w, surface->h);
	SDL_FreeSurface(surface);
	return texnum != 0;
}

TexNum GL3Frontend::uploadTexture (const unsigned char* pixels, int w, int h) const
{
	TexNum texnum;
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_checkError();
	return texnum;
}

void GL3Frontend::makeScreenshot (const std::string& filename)
{
#ifndef EMSCRIPTEN
	const int bytesPerPixel = 3;
	ScopedPtr<GLubyte> pixels(new GLubyte[bytesPerPixel * _width * _height]);
	int rowPack;
	glGetIntegerv(GL_PACK_ALIGNMENT, &rowPack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_PACK_ALIGNMENT, rowPack);

	ScopedPtr<SDL_Surface> surface(SDL_CreateRGBSurface(SDL_SWSURFACE, _width, _height, 24,
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
		memcpy((uint8 *) surface->pixels + surface->pitch * y, (uint8 *) pixels + pitch * (_height - y - 1), pitch);
	const std::string fullFilename = FS.getAbsoluteWritePath() + filename + "-" + dateutil::getDateString() + ".png";
	IMG_SavePNG(surface, fullFilename.c_str());
#endif
}

void GL3Frontend::updateViewport (int x, int y, int width, int height)
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

	glViewport(_viewPort.x, _viewPort.y, _viewPort.w, _viewPort.h);
	const GLdouble _w = static_cast<GLdouble>(_viewPort.w);
	const GLdouble _h = static_cast<GLdouble>(_viewPort.h);
	_projectionMatrix = glm::ortho(0.0, _w, _h, 0.0, -1.0, 1.0);
	GL_checkError();
}

void GL3Frontend::renderBegin ()
{
	SDL_GL_MakeCurrent(_window, _context);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	_currentBatch = 0;
	_drawCalls = 0;
}

void GL3Frontend::renderEnd ()
{
	renderBatches();
#ifdef DEBUG
	debug(LOG_CLIENT, String::format("%i drawcalls", _drawCalls));
#endif
	SDL_GL_SwapWindow(_window);
	GL_checkError();
}
