#include "GL3Frontend.h"
#include "textures/TextureCoords.h"
#include "common/Log.h"
#include "common/System.h"
#include "common/FileSystem.h"
#include <SDL.h>

GL3Frontend::GL3Frontend (std::shared_ptr<IConsole> console) :
		AbstractGLFrontend(console), _vao(0u), _vbo(0u), _waterNoise(0)
{
}

GL3Frontend::~GL3Frontend ()
{
}

bool GL3Frontend::renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& waterLineColor)
{
	renderBatches();
	const float width = _fbo.rect().w;
	const float height = _fbo.rect().h;
	const float xTexCoord = x / width;
	const float xTexCoord2 = xTexCoord + w / width;
	const float yTexCoord = 1.0 - y / height;
	const float yTexCoord2 = 1.0 - (y + h) / height;

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
	renderTexture(texCoords, x, y, w, h, 0, 1.0f, _renderTargetTexture, _waterNoise);
	Log::trace(LOG_CLIENT, "x: %i, y: %i, w: %i, h: %i, fbo(%f, %f), tex(%f:%f:%f:%f)", x, y, w, h, width, height, xTexCoord, yTexCoord, xTexCoord2, yTexCoord2);
	_waterShader.activate();
	if (_waterShader.hasUniform("u_watercolor"))
		_waterShader.setUniform4fv("u_watercolor", fillColor, 0, 4);
	renderBatchesWithShader(_waterShader);
	renderLine(x, y - 1, x + w, y - 1, waterLineColor);
	return true;
}

void GL3Frontend::renderBatches()
{
	renderBatchesWithShader(_shader);
}

void GL3Frontend::renderBatchesWithShader (Shader& shader)
{
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

	renderBatchBuffers();

	glBindVertexArray(0);
	shader.deactivate();
}

void GL3Frontend::initRenderer () {
	Log::info(LOG_CLIENT, "init opengl renderer");
	_context = SDL_GL_CreateContext(_window);
	ExtGLLoadFunctions();

	glClearColor(0, 0, 0, 0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		Log::error(LOG_CLIENT, "Failed to load the main shader");
		System.exit("Failed to load the main shader", 1);
	}
	if (!_waterShader.loadProgram("water")) {
		Log::error(LOG_CLIENT, "Failed to load the water shader");
	}
	_waterShader.activate();
	if (_waterShader.hasUniform("u_texture"))
		_waterShader.setUniformi("u_texture", 0);
	if (_waterShader.hasUniform("u_normals"))
		_waterShader.setUniformi("u_normals", 1);
	_waterShader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, x)));
	_waterShader.enableVertexAttributeArray("a_pos");
	_waterShader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, u)));
	_waterShader.enableVertexAttributeArray("a_texcoord");
	_waterShader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, c)));
	_waterShader.enableVertexAttributeArray("a_color");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	_waterShader.deactivate();

	_shader.activate();
	if (_shader.hasUniform("u_texture"))
		_shader.setUniformi("u_texture", 0);
	if (_shader.hasUniform("u_normals"))
		_shader.setUniformi("u_normals", 1);
	_shader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, x)));
	_shader.enableVertexAttributeArray("a_pos");
	_shader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, u)));
	_shader.enableVertexAttributeArray("a_texcoord");
	_shader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, c)));
	_shader.enableVertexAttributeArray("a_color");
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	_shader.deactivate();

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	SDL_Surface *textureSurface = loadTextureIntoSurface("waternoise");
	if (textureSurface == nullptr) {
		Log::error(LOG_CLIENT, "Could not load the water noise");
	} else {
		_waterNoise = uploadTexture(static_cast<unsigned char *>(textureSurface->pixels), textureSurface->w, textureSurface->h);
		SDL_FreeSurface(textureSurface);
		Log::info(LOG_CLIENT, "Uploaded water noise with texnum %u", _waterNoise);
	}
}
