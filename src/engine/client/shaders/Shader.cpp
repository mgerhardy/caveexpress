#include "Shader.h"
#include "engine/common/Logger.h"
#include "engine/common/FileSystem.h"
#include "engine/common/ConfigManager.h"

#define MAX_SHADER_VAR_NAME 128

Shader::Shader () :
		_program(0), _initialized(false), _active(false), _time(0)
{
	for (int i = 0; i < SHADER_MAX; ++i) {
		_shader[i] = 0;
	}
}

Shader::~Shader ()
{
	if (!GLContext::get().areShadersSupported())
		return;

#ifdef USE_SHADERS
	for (int i = 0; i < SHADER_MAX; ++i) {
		GLContext::get().ctx_glDeleteShader(_shader[i]);
	}
	GLContext::get().ctx_glDeleteProgram(_program);
#endif
}

void Shader::update (uint32_t deltaTime)
{
	_time += deltaTime;
}

bool Shader::load (const std::string& filename, const std::string& source, ShaderType shaderType)
{
	if (!GLContext::get().areShadersSupported())
		return false;

#ifdef USE_SHADERS
	const GLenum glType = shaderType == SHADER_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	GL_checkError();

	_shader[shaderType] = GLContext::get().ctx_glCreateShader(glType);
	const char *s = source.c_str();
	GLContext::get().ctx_glShaderSource(_shader[shaderType], 1, (const GLchar**) &s, nullptr);
	GLContext::get().ctx_glCompileShader(_shader[shaderType]);

	GLint status;
	GLContext::get().ctx_glGetShaderiv(_shader[shaderType], GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE || glGetError() != GL_NO_ERROR) {
		GLint infoLogLength;
		GLContext::get().ctx_glGetShaderiv(_shader[shaderType], GL_INFO_LOG_LENGTH, &infoLogLength);

		ScopedPtr<GLchar> strInfoLog(new GLchar[infoLogLength + 1]);
		GLContext::get().ctx_glGetShaderInfoLog(_shader[shaderType], infoLogLength, nullptr, strInfoLog);
		std::string errorLog(strInfoLog, static_cast<std::size_t>(infoLogLength));

		std::string strShaderType;
		switch (glType) {
		case GL_VERTEX_SHADER:
			strShaderType = "vertex";
			break;
		case GL_FRAGMENT_SHADER:
			strShaderType = "fragment";
			break;
		default:
			strShaderType = "unknown";
			break;
		}

		error(LOG_CLIENT, "compile failure in " + filename + " (type: " + strShaderType + ") shader:\n" + errorLog);
	}

	return true;
#else
	return false;
#endif
}

bool Shader::loadFromFile (const std::string& filename, ShaderType shaderType)
{
	FilePtr filePtr = FS.getFile(FS.getShaderDir() + filename);
	char *buffer;
	int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		error(LOG_CLIENT, "could not load shader " + filename);
		return false;
	}

	const std::string src = getSource(shaderType, buffer, fileLen);
	return load(filename, src, shaderType);
}

std::string Shader::getSource (ShaderType shaderType, const char *buffer, int len)
{
	std::string src;
#ifdef GL_ES_VERSION_2_0
	src.append("#version 120\n");
	if (shaderType == SHADER_FRAGMENT) {
		src.append("#ifdef GL_ES\n");
		src.append("precision mediump float;\n");
		src.append("precision mediump int;\n");
		src.append("#endif\n");
	}
#else
	src.append("#version 120\n#define lowp\n#define mediump\n#define highp\n");
#endif

	std::string append(buffer, len);

	const std::string include = "#include";
	int index = 0;
	for (std::string::iterator i = append.begin(); i != append.end(); ++i, ++index) {
		const char *c = &append[index];
		if (*c != '#') {
			src.append(c, 1);
			continue;
		}
		if (strncmp(include.c_str(), c, include.length())) {
			src.append(c, 1);
			continue;
		}
		for (; i != append.end(); ++i, ++index) {
			const char *cStart = &append[index];
			if (*cStart != '"')
				continue;

			++index;
			++i;
			for (; i != append.end(); ++i, ++index) {
				const char *cEnd = &append[index];
				if (*cEnd != '"')
					continue;

				const std::string includeFile(cStart + 1, cEnd);
				FilePtr filePtr = FS.getFile(FS.getShaderDir() + includeFile);
				char *includeBuffer;
				const int includeLen = filePtr->read((void **) &includeBuffer);
				ScopedArrayPtr<char> p(includeBuffer);
				if (!includeBuffer || includeLen <= 0) {
					error(LOG_CLIENT, "could not load shader include " + includeFile);
					break;
				}
				src.append(includeBuffer, includeLen);
				break;
			}
			break;
		}
	}

	return src;
}

bool Shader::loadProgram (const std::string& filename)
{
	if (!GLContext::get().areShadersSupported())
		return false;

	const bool vertex = loadFromFile(filename + "_vs.glsl", SHADER_VERTEX);
	if (!vertex)
		return false;

	const bool fragment = loadFromFile(filename + "_fs.glsl", SHADER_FRAGMENT);
	if (!fragment)
		return false;

	createProgramFromShaders();
	fetchAttributes();
	fetchUniforms();
	const bool success = _program != 0;
	if (success) {
		info(LOG_CLIENT, "loaded shader: " + filename);
	}
	_initialized = success;
	return success;
}

void Shader::fetchUniforms ()
{
#ifdef USE_SHADERS
	char name[MAX_SHADER_VAR_NAME];
	int numUniforms = 0;
	GLContext::get().ctx_glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &numUniforms);
	GL_checkError();

	_uniforms.clear();
	for (int i = 0; i < numUniforms; i++) {
		GLsizei length;
		GLint size;
		GLenum type;
		GLContext::get().ctx_glGetActiveUniform(_program, i, MAX_SHADER_VAR_NAME - 1, &length, &size, &type, name);
		const int location = GLContext::get().ctx_glGetUniformLocation(_program, name);
		_uniforms[name] = location;
	}
#endif
}

void Shader::fetchAttributes ()
{
#ifdef USE_SHADERS
	char name[MAX_SHADER_VAR_NAME];
	int numAttributes = 0;
	GLContext::get().ctx_glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
	GL_checkError();

	_attributes.clear();
	for (int i = 0; i < numAttributes; i++) {
		GLsizei length;
		GLint size;
		GLenum type;
		GLContext::get().ctx_glGetActiveAttrib(_program, i, MAX_SHADER_VAR_NAME - 1, &length, &size, &type, name);
		const int location = GLContext::get().ctx_glGetAttribLocation(_program, name);
		_attributes[name] = location;
	}
#endif
}

void Shader::createProgramFromShaders ()
{
#ifdef USE_SHADERS
	GL_checkError();
	GLint status;
	_program = GLContext::get().ctx_glCreateProgram();
	GL_checkError();

	const GLuint vert = _shader[SHADER_VERTEX];
	const GLuint frag = _shader[SHADER_FRAGMENT];

	GLContext::get().ctx_glAttachShader(_program, vert);
	GLContext::get().ctx_glAttachShader(_program, frag);
	GL_checkError();

	GLContext::get().ctx_glLinkProgram(_program);
	GLContext::get().ctx_glGetProgramiv(_program, GL_LINK_STATUS, &status);
	GL_checkError();
	if (status != GL_TRUE) {
		GLint infoLogLength;
		GLContext::get().ctx_glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		GLContext::get().ctx_glGetProgramInfoLog(_program, infoLogLength, nullptr, strInfoLog);
		error(LOG_CLIENT, String::format("linker failure: %s", strInfoLog));
		GLContext::get().ctx_glDeleteProgram(_program);
		_program = 0;
		delete[] strInfoLog;
	}
#endif
}

int Shader::getAttributeLocation (const std::string& name) const
{
	ShaderVariables::const_iterator i = _attributes.find(name);
	if (i == _attributes.end()) {
		error(LOG_CLIENT, "can't find attribute " + name);
		return -1;
	}
	return i->second;
}

int Shader::getUniformLocation (const std::string& name) const
{
	ShaderVariables::const_iterator i = _uniforms.find(name);
	if (i == _uniforms.end()) {
		error(LOG_CLIENT, "can't find uniform " + name);
		return -1;
	}
	return i->second;
}

void Shader::setProjectionMatrix (const glm::mat4& projectionMatrix)
{
	_projectionMatrix = projectionMatrix;
}

void Shader::setModelViewMatrix (const glm::mat4& modelViewMatrix)
{
	_modelViewMatrix = modelViewMatrix;
}

bool Shader::activate () const
{
	if (!GLContext::get().areShadersSupported())
		return false;

	if (!Config.isShader()) {
		return false;
	}

#ifdef USE_SHADERS
	GLContext::get().ctx_glUseProgram(_program);
	GL_checkError();
#endif
	_active = true;

	return true;
}

void Shader::deactivate () const
{
	if (!_active) {
		return;
	}

#ifdef USE_SHADERS
	GLContext::get().ctx_glUseProgram(0);
	GL_checkError();
#endif
	_active = false;
	_time = 0;
}

ShaderScope::ShaderScope (const Shader& shader) :
		_shader(shader)
{
	_shader.activate();
}

ShaderScope::~ShaderScope ()
{
	_shader.deactivate();
}

