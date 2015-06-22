#include "GL3Frontend.h"
#include "textures/TextureCoords.h"
#include "common/Logger.h"
#include "common/System.h"
#include "common/FileSystem.h"
#include <SDL_image.h>

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

static Vertex _vertices[MAXNUMVERTICES];
static int _currentVertexIndex;
static Batch _batches[MAX_BATCHES];
static int _currentBatch;

GL3Frontend::GL3Frontend (SharedPtr<IConsole> console) :
		SDLFrontend(console), _currentTexture(-1), _rx(1.0f), _ry(1.0f), _vao(0u), _vbo(0u), _renderTargetTexture(0), _white(0), _alpha(0), _drawCalls(0)
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

RenderTarget* GL3Frontend::renderToTexture (int x, int y, int w, int h)
{
	// render the current batches and then start to write to the fbo texture
	renderBatches();
	static RenderTarget target;
	target.fbo = &_fbo;
	target.fbo->bind(x, y, w, h);
	return &target;
}

void GL3Frontend::bindTargetTexture(RenderTarget* target)
{
}

void GL3Frontend::unbindTargetTexture (RenderTarget* target)
{
}

bool GL3Frontend::renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& waterLineColor)
{
	renderBatches();
	const float width = _fbo.rect().w;
	const float height = _fbo.rect().h;
	const float xf = x;
	const float yf = y;
	const float xTexCoord = xf / width;
	const float xTexCoord2 = xTexCoord + w / width;
	const float yTexCoord = 1.0 - yf / height;
	const float yTexCoord2 = 1.0 - (yf + h) / height;

	float tex[8];

	tex[0] = xTexCoord;
	tex[1] = yTexCoord;

	tex[2] = xTexCoord2;
	tex[3] = yTexCoord;

	tex[4] = xTexCoord2;
	tex[5] = yTexCoord2;

	tex[6] = xTexCoord;
	tex[7] = yTexCoord2;

	const TextureCoords texCoords(tex);
	renderTexture(texCoords, x, y, w, h, 0, 1.0f, _renderTargetTexture, _alpha);
	trace(LOG_CLIENT, String::format("x: %i, y: %i, w: %i, h: %i, fbo(%f, %f), tex(%f:%f:%f:%f)", x, y, w, h, width, height, xTexCoord, yTexCoord, xTexCoord2, yTexCoord2));
	_waterShader.activate();
	if (_waterShader.hasUniform("u_watercolor"))
		_waterShader.setUniform4fv("u_watercolor", fillColor, 0, 4);
	renderBatchesWithShader(_waterShader);
	return true;
}

bool GL3Frontend::renderTarget (RenderTarget* target)
{
	disableRenderTarget(target);
	const TextureRect& r = target->fbo->rect();
	const TextureCoords texCoords(r, r.w, r.h, false, true);
	renderTexture(texCoords, r.x, r.y, r.w, r.h, 0, 1.0, _renderTargetTexture, _alpha);
	return true;
}

bool GL3Frontend::disableRenderTarget (RenderTarget* target)
{
	// render the current batches to the fbo texture - and unbind the fbo
	renderBatches();
	target->fbo->unbind();
	return true;
}

void GL3Frontend::renderImage (Texture* texture, int x, int y, int w, int h, int16_t angle, float alpha)
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
void GL3Frontend::renderTexture(const TextureCoords& texCoords, int x, int y, int w, int h, int16_t angle, float alpha, GLuint texnum, GLuint normaltexnum)
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
	batch.texnum = texnum;
	batch.normaltexnum = normaltexnum;
	batch.vertexCount += 6;

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

void GL3Frontend::renderBatches()
{
	renderBatchesWithShader(_shader);
}

void GL3Frontend::renderBatchesWithShader (Shader& shader)
{
	if (_batches[0].texnum == 0)
		return;
	shader.activate();
	if (shader.hasUniform("u_projection"))
		shader.setUniformMatrix("u_projection", _projectionMatrix, false);
	if (shader.hasUniform("u_time"))
		shader.setUniformi("u_time", _time);
	if (shader.hasUniform("u_screenres"))
		shader.setUniformf("u_screenres", _width, _height);
	if (shader.hasUniform("u_mousepos")) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		shader.setUniformf("u_mousepos", x, y);
	}
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _currentVertexIndex, _vertices, GL_DYNAMIC_DRAW);
	bool scissorActive = false;
	for (int i = 0; i <= _currentBatch; ++i) {
		Batch& b = _batches[i];
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
		if (_currentTexture != b.texnum) {
			_currentTexture = b.texnum;
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, b.normaltexnum);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, b.texnum);
		}
		glDrawArrays(b.type, b.vertexIndexStart, b.vertexCount);
	}
	if (scissorActive)
		glDisable(GL_SCISSOR_TEST);
	_drawCalls += _currentBatch;
	_currentVertexIndex = 0;
	glBindVertexArray(0);
	const SDL_Rect scissorRect = _batches[_currentBatch].scissorRect;
	_currentBatch = 0;
	memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
	_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
	_batches[_currentBatch].scissorRect = scissorRect;
	shader.deactivate();
	GL_checkError();
}

void GL3Frontend::flushBatch (int type, GLuint texnum, int vertexAmount)
{
	if (_currentVertexIndex + vertexAmount >= MAXNUMVERTICES)
		renderBatches();
	const Batch& b = _batches[_currentBatch];
	if (b.type == type && b.texnum == texnum)
		return;
	startNewBatch();
	_batches[_currentBatch].type = type;
}

void GL3Frontend::startNewBatch ()
{
	const Batch& b = _batches[_currentBatch];
	if (_currentVertexIndex - b.vertexIndexStart == 0)
		return;

	if (b.vertexCount == 0)
		return;

	if (_currentVertexIndex + 6 >= MAXNUMVERTICES)
		return;

	++_currentBatch;
	if (_currentBatch >= MAX_BATCHES) {
		debug(LOG_CLIENT, "render the batches because the max batch count was exceeded");
		renderBatches();
		return;
	}
	memset(&_batches[_currentBatch], 0, sizeof(_batches[_currentBatch]));
	_batches[_currentBatch].vertexIndexStart = _currentVertexIndex;
	_batches[_currentBatch].scissorRect = _batches[_currentBatch - 1].scissorRect;
}

void GL3Frontend::enableScissor (int x, int y, int width, int height)
{
	const int lowerLeft = _fbo.isBound() ? y : std::max(0, getHeight() - y - height);
	startNewBatch();
	_batches[_currentBatch].scissor = true;
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
	_batches[_currentBatch].scissor = false;
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

	unsigned char alpha[16];
	memset(alpha, 0x00, sizeof(alpha));
	_alpha = uploadTexture(alpha, 2, 2);

	memset(_batches, 0, sizeof(_batches));
	_currentVertexIndex = 0;

	if (!_shader.loadProgram("main")) {
		error(LOG_CLIENT, "Failed to load the main shader");
		System.exit("Failed to load the main shader", 1);
	}
	if (!_waterShader.loadProgram("water")) {
		error(LOG_CLIENT, "Failed to load the water shader");
		System.exit("Failed to load the water shader", 1);
	}
	_waterShader.activate();
	if (_waterShader.hasUniform("u_texture"))
		_waterShader.setUniformi("u_texture", 0);
	if (_waterShader.hasUniform("u_normals"))
		_waterShader.setUniformi("u_normals", 1);
	_waterShader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, x)));
	_waterShader.enableVertexAttributeArray("a_pos");
	_waterShader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, u)));
	_waterShader.enableVertexAttributeArray("a_texcoord");
	_waterShader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, c)));
	_waterShader.enableVertexAttributeArray("a_color");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	_waterShader.deactivate();

	_shader.activate();
	if (_shader.hasUniform("u_texture"))
		_shader.setUniformi("u_texture", 0);
	if (_shader.hasUniform("u_normals"))
		_shader.setUniformi("u_normals", 1);
	_shader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, x)));
	_shader.enableVertexAttributeArray("a_pos");
	_shader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, u)));
	_shader.enableVertexAttributeArray("a_texcoord");
	_shader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_OFFSET(offsetof(Vertex, c)));
	_shader.enableVertexAttributeArray("a_color");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	_shader.deactivate();

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
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
	const TexNum texnum = texture->getData()->texnum;
	if (_currentTexture == texnum)
		return;
	_currentTexture = texnum;
	Batch& batch = _batches[_currentBatch];
	batch.texnum = texnum;
	batch.normaltexnum = texture->getData()->normalnum;
	glBindTexture(GL_TEXTURE_2D, _currentTexture);
}

void GL3Frontend::renderFilledRect (int x, int y, int w, int h, const Color& color)
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
	batch.texnum = _white;
	batch.normaltexnum = _alpha;
	batch.scissor = false;
	batch.scissorRect = {0, 0, 0, 0};
	batch.vertexCount += 6;

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

void GL3Frontend::renderRect (int x, int y, int w, int h, const Color& color)
{
	renderFilledRect(x, y, w, 1, color);
	renderFilledRect(x, y + h - 1, w, 1, color);
	renderFilledRect(x, y + 1, 1, h - 2, color);
	renderFilledRect(x + w - 1, y + 1, 1, h - 2, color);
}

void GL3Frontend::renderLine (int x1, int y1, int x2, int y2, const Color& color)
{
	flushBatch(GL_LINES, _white, 2);
	Batch& batch = _batches[_currentBatch];
	batch.texnum = _white;
	batch.normaltexnum = _alpha;
	batch.scissor = false;
	batch.scissorRect = {0, 0, 0, 0};
	batch.vertexCount += 2;

	Vertex v(color);
	v.x = x1 * _rx;
	v.y = y1 * _ry;
	_vertices[_currentVertexIndex++] = v;

	v.x = x2 * _rx;
	v.y = y2 * _ry;
	_vertices[_currentVertexIndex++] = v;
}

void GL3Frontend::destroyTexture (TextureData *data)
{
	if (data->texnum != _white && data->texnum != _alpha && data->normalnum != _white && data->normalnum != _alpha) {
		glDeleteTextures(1, &data->texnum);
		glDeleteTextures(1, &data->normalnum);
		GL_checkError();
	}
}

SDL_Surface* GL3Frontend::loadTextureIntoSurface(const std::string& filename) {
	const std::string file = FS.getFile(FS.getPicsDir() + filename + ".png")->getName();
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

bool GL3Frontend::loadTexture (Texture *texture, const std::string& filename)
{
	SDL_Surface* textureSurface = loadTextureIntoSurface(filename);
	if (textureSurface == nullptr) {
		error(LOG_CLIENT, "could not load the file: " + filename);
		return false;
	}
	SDL_Surface* normalSurface = loadTextureIntoSurface(filename + "_n");
	TextureData* data = new TextureData();
	data->texnum = uploadTexture(static_cast<unsigned char*>(textureSurface->pixels), textureSurface->w, textureSurface->h);
	if (normalSurface) {
		data->normalnum = uploadTexture(static_cast<unsigned char*>(normalSurface->pixels), normalSurface->w, normalSurface->h);
		info(LOG_CLIENT, "load normal map for: " + filename);
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
