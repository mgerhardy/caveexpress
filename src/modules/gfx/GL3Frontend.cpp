#include "GL3Frontend.h"
#include "gfx/SDLFrontend.h"
#include "imgui_impl_sdl2.h"
#include "textures/TextureCoords.h"
#include "common/Log.h"
#include "common/System.h"
#include "common/FileSystem.h"
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

GL3Frontend::GL3Frontend (std::shared_ptr<IConsole> console) :
		AbstractGLFrontend(console), _vao(0u), _vbo(0u), _waterNoise(0)
{
}

GL3Frontend::~GL3Frontend ()
{
}

bool GL3Frontend::renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& lineColor, const vec2 &offsets)
{
	renderBatches();
	const float width = _fbo.rect().w;
	const float height = _fbo.rect().h;
	const float xTexCoord = x / width;
	const float xTexCoord2 = xTexCoord + w / width;
	const float yTexCoord = 1.0f - y / height;
	const float yTexCoord2 = 1.0f - (y + h) / height;

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
	Log::trace(LOG_GFX, "o: %4.3f, %4.3f  x: %i, y: %i, w: %i, h: %i, fbo(%f, %f), tex(%f:%f:%f:%f)",
		offsets.x, offsets.y, x, y, w, h, width, height, xTexCoord, yTexCoord, xTexCoord2, yTexCoord2);
	_waterShader.activate();
	if (_waterShader.hasUniform("u_watercolor"))
		_waterShader.setUniform4fv("u_watercolor", &fillColor.rgba[0], 0, 4);
	if (_waterShader.hasUniform("u_offsets"))
		_waterShader.setUniform2fv("u_offsets", &offsets.x, 0, 2);
	renderBatchesWithShader(_waterShader);
	renderLine(x, y - 1, x + w, y - 1, lineColor);
	return true;
}

bool GL3Frontend::renderHeatHaze (int x, int y, int w, int h)
{
	if (!_lavaShader.isInitialized() || _waterNoise == 0 || _renderTargetTexture == 0)
		return false;

	renderBatches();
	const float width = _fbo.rect().w;
	const float height = _fbo.rect().h;
	if (width <= 0.0f || height <= 0.0f)
		return false;

	const float xTexCoord = x / width;
	const float xTexCoord2 = xTexCoord + w / width;
	const float yTexCoord = 1.0f - y / height;
	const float yTexCoord2 = 1.0f - (y + h) / height;

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
	_lavaShader.activate();
	if (_lavaShader.hasUniform("u_bandv"))
		_lavaShader.setUniformf("u_bandv", yTexCoord, yTexCoord2);
	if (_lavaShader.hasUniform("u_bandu"))
		_lavaShader.setUniformf("u_bandu", xTexCoord, xTexCoord2);
	if (_lavaShader.hasUniform("u_fadeuv"))
		_lavaShader.setUniformf("u_fadeuv", 28.0f / width, 32.0f / height);
	renderBatchesWithShader(_lavaShader);
	return true;
}

void GL3Frontend::renderBatches()
{
	renderBatchesWithShader(_shader);
	Super::renderBatches();
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
	if (shader.hasUniform("u_lightcount"))
		shader.setUniformi("u_lightcount", _lightCount);
	if (shader.hasUniform("u_lights") && _lightCount > 0) {
		float packed[MAX_RENDER_LIGHTS * 4];
		for (int i = 0; i < _lightCount; ++i) {
			packed[i * 4 + 0] = _lights[i].x * _rx;
			packed[i * 4 + 1] = _lights[i].y * _ry;
			packed[i * 4 + 2] = _lights[i].radius * _rx;
			packed[i * 4 + 3] = _lights[i].intensity;
		}
		shader.setUniform4fv("u_lights", packed, 0, _lightCount * 4);
	}
	if (shader.hasUniform("u_lightcolors") && _lightCount > 0) {
		float colors[MAX_RENDER_LIGHTS * 3];
		for (int i = 0; i < _lightCount; ++i) {
			colors[i * 3 + 0] = _lights[i].r;
			colors[i * 3 + 1] = _lights[i].g;
			colors[i * 3 + 2] = _lights[i].b;
		}
		shader.setUniform3fv("u_lightcolors", colors, 0, _lightCount * 3);
	}
	if (shader.hasUniform("u_lightfalloff") && _lightCount > 0) {
		float falloff[MAX_RENDER_LIGHTS];
		for (int i = 0; i < _lightCount; ++i)
			falloff[i] = _lights[i].falloff;
		shader.setUniform1fv("u_lightfalloff", falloff, 0, _lightCount);
	}
	glBindVertexArray(_vao);
	GL_checkError();
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	GL_checkError();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _currentVertexIndex, _vertices, GL_DYNAMIC_DRAW);
	GL_checkError();

	renderBatchBuffers();

	glBindVertexArray(0);
	GL_checkError();
	shader.deactivate();
}

void GL3Frontend::initRenderer () {
	Log::info(LOG_GFX, "init opengl renderer");
	AbstractGLFrontend::initRenderer();

	glGenVertexArrays(1, &_vao);
	GL_checkError();
	glGenBuffers(1, &_vbo);
	GL_checkError();

	glBindVertexArray(_vao);
	GL_checkError();
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	GL_checkError();

	if (!_shader.loadProgram("main")) {
		Log::error(LOG_GFX, "Failed to load the main shader");
		System.exit("Failed to load the main shader", 1);
	}
	if (!_waterShader.loadProgram("water")) {
		Log::error(LOG_GFX, "Failed to load the water shader");
	} else {
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
		GL_checkError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL_checkError();
		_waterShader.deactivate();
	}

	if (!_lavaShader.loadProgram("lava")) {
		Log::error(LOG_GFX, "Failed to load the lava shader");
	} else {
		glBindVertexArray(_vao);
		GL_checkError();
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		GL_checkError();
		_lavaShader.activate();
		if (_lavaShader.hasUniform("u_texture"))
			_lavaShader.setUniformi("u_texture", 0);
		if (_lavaShader.hasUniform("u_normals"))
			_lavaShader.setUniformi("u_normals", 1);
		_lavaShader.setVertexAttribute("a_pos", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, x)));
		_lavaShader.enableVertexAttributeArray("a_pos");
		_lavaShader.setVertexAttribute("a_texcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, u)));
		_lavaShader.enableVertexAttributeArray("a_texcoord");
		_lavaShader.setVertexAttribute("a_color", 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, c)));
		_lavaShader.enableVertexAttributeArray("a_color");
		glBindVertexArray(0);
		GL_checkError();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL_checkError();
		_lavaShader.deactivate();
	}

	glBindVertexArray(_vao);
	GL_checkError();
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	GL_checkError();
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
	if (_shader.hasAttribute("a_normalcoord")) {
		_shader.setVertexAttribute("a_normalcoord", 2, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, nu)));
		_shader.enableVertexAttributeArray("a_normalcoord");
	}
	if (_shader.hasAttribute("a_lit")) {
		_shader.setVertexAttribute("a_lit", 1, GL_FLOAT, false, sizeof(Vertex), GL_CALC_OFFSET(offsetof(Vertex, lit)));
		_shader.enableVertexAttributeArray("a_lit");
	}
	glBindVertexArray(0);
	GL_checkError();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL_checkError();
	_shader.deactivate();

	glActiveTexture(GL_TEXTURE1);
	GL_checkError();
#ifndef HAVE_GLES
	glEnable(GL_TEXTURE_2D);
	GL_checkError();
#endif

	glActiveTexture(GL_TEXTURE0);
	GL_checkError();
#ifndef HAVE_GLES
	glEnable(GL_TEXTURE_2D);
	GL_checkError();
#endif

	SDL_Surface *textureSurface = loadTextureIntoSurface("waternoise");
	if (textureSurface == nullptr) {
		Log::error(LOG_GFX, "Could not load the water noise");
	} else {
		_waterNoiseW = textureSurface->w;
		_waterNoiseH = textureSurface->h;
		_waterNoise = uploadTexture(static_cast<unsigned char *>(textureSurface->pixels), textureSurface->w, textureSurface->h);
		SDL_FreeSurface(textureSurface);
		Log::info(LOG_GFX, "Uploaded water noise with texnum %u", _waterNoise);
	}

	ImGui_ImplSDL2_InitForOpenGL(_window, _context);
#ifdef HAVE_GLES
	ImGui_ImplOpenGL3_Init("#version 300 es");
#else
	ImGui_ImplOpenGL3_Init(nullptr);
#endif
}

void GL3Frontend::newFrameImGui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
}

void GL3Frontend::renderImGui() {
	if (ImGui::GetCurrentContext() == nullptr)
		return;
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GL3Frontend::shutdownImGui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
}
