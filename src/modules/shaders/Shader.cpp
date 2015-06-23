#include "Shader.h"
#include "common/Log.h"
#include "common/FileSystem.h"

#define MAX_SHADER_VAR_NAME 128

Shader::Shader () :
		_program(0), _initialized(false), _active(false), _time(0)
{
	for (int i = 0; i < SHADER_MAX; ++i) {
		_shader[i] = 0;
	}
	_name = "unknown";
}

Shader::~Shader ()
{
	if (_program == 0)
		return;
	for (int i = 0; i < SHADER_MAX; ++i) {
		glDeleteShader(_shader[i]);
	}
	glDeleteProgram(_program);
}

void Shader::update (uint32_t deltaTime)
{
	_time += deltaTime;
}

bool Shader::load (const std::string& filename, const std::string& source, ShaderType shaderType)
{
	if (glCreateShader == nullptr)
		return false;
	const GLenum glType = shaderType == SHADER_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	GL_checkError();

	_shader[shaderType] = glCreateShader(glType);
	const char *s = source.c_str();
	glShaderSource(_shader[shaderType], 1, (const GLchar**) &s, nullptr);
	glCompileShader(_shader[shaderType]);

	GLint status;
	glGetShaderiv(_shader[shaderType], GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE || glGetError() != GL_NO_ERROR) {
		GLint infoLogLength;
		glGetShaderiv(_shader[shaderType], GL_INFO_LOG_LENGTH, &infoLogLength);

		ScopedPtr<GLchar> strInfoLog(new GLchar[infoLogLength + 1]);
		glGetShaderInfoLog(_shader[shaderType], infoLogLength, nullptr, strInfoLog);
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

		Log::error(LOG_CLIENT, "compile failure in %s (type: %s) shader:\n%s", filename.c_str(), strShaderType.c_str(), errorLog.c_str());
		return false;
	}

	return true;
}

bool Shader::loadFromFile (const std::string& filename, ShaderType shaderType)
{
	FilePtr filePtr = FS.getFile(FS.getShaderDir() + filename);
	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_CLIENT, "could not load shader %s", filename.c_str());
		return false;
	}

	const std::string& src = getSource(shaderType, buffer, fileLen);
	return load(filename, src, shaderType);
}

std::string Shader::getSource (ShaderType shaderType, const char *buffer, int len)
{
	std::string src;
#ifdef GL_ES_VERSION_2_0
	src.append("#version 300\n");
#else
	src.append("#version 130\n");
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
					Log::error(LOG_CLIENT, "could not load shader include %s", includeFile.c_str());
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
	_name = filename;

	const bool vertex = loadFromFile(filename + ".vert", SHADER_VERTEX);
	if (!vertex)
		return false;

	const bool fragment = loadFromFile(filename + ".frag", SHADER_FRAGMENT);
	if (!fragment)
		return false;

	createProgramFromShaders();
	fetchAttributes();
	fetchUniforms();
	const bool success = _program != 0;
	if (success) {
		Log::info(LOG_CLIENT, "loaded shader: %s", filename.c_str());
	}
	_initialized = success;
	return success;
}

void Shader::fetchUniforms ()
{
	char name[MAX_SHADER_VAR_NAME];
	int numUniforms = 0;
	glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &numUniforms);
	GL_checkError();

	_uniforms.clear();
	for (int i = 0; i < numUniforms; i++) {
		GLsizei length;
		GLint size;
		GLenum type;
		glGetActiveUniform(_program, i, MAX_SHADER_VAR_NAME - 1, &length, &size, &type, name);
		const int location = glGetUniformLocation(_program, name);
		_uniforms[name] = location;
		Log::debug(LOG_CLIENT, "uniform %s found at location %i in shader %s", name, location, _name.c_str());
	}
}

void Shader::fetchAttributes ()
{
	char name[MAX_SHADER_VAR_NAME];
	int numAttributes = 0;
	glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
	GL_checkError();

	_attributes.clear();
	for (int i = 0; i < numAttributes; i++) {
		GLsizei length;
		GLint size;
		GLenum type;
		glGetActiveAttrib(_program, i, MAX_SHADER_VAR_NAME - 1, &length, &size, &type, name);
		const int location = glGetAttribLocation(_program, name);
		_attributes[name] = location;
		Log::debug(LOG_CLIENT, "attribute %s found at location %i in shader %s", name, location, _name.c_str());
	}
}

void Shader::createProgramFromShaders ()
{
	GL_checkError();
	GLint status;
	_program = glCreateProgram();
	GL_checkError();

	const GLuint vert = _shader[SHADER_VERTEX];
	const GLuint frag = _shader[SHADER_FRAGMENT];

	glAttachShader(_program, vert);
	glAttachShader(_program, frag);
	GL_checkError();

	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &status);
	GL_checkError();
	if (status != GL_TRUE) {
		GLint infoLogLength;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(_program, infoLogLength, nullptr, strInfoLog);
		Log::error(LOG_CLIENT, "linker failure: %s", strInfoLog);
		glDeleteProgram(_program);
		_program = 0;
		delete[] strInfoLog;
	}
}

int Shader::getAttributeLocation (const std::string& name) const
{
	ShaderVariables::const_iterator i = _attributes.find(name);
	if (i == _attributes.end()) {
		Log::error(LOG_CLIENT, "can't find attribute %s in shader %s", name.c_str(), _name.c_str());
		return -1;
	}
	return i->second;
}

int Shader::getUniformLocation (const std::string& name) const
{
	ShaderVariables::const_iterator i = _uniforms.find(name);
	if (i == _uniforms.end()) {
		Log::error(LOG_CLIENT, "can't find uniform %s in shader %s", name.c_str(), _name.c_str());
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
	if (!_initialized)
		return false;
	if (_active)
		return false;
	glUseProgram(_program);
	GL_checkError();
	_active = true;

	return true;
}

void Shader::deactivate () const
{
	if (!_active) {
		return;
	}

	glUseProgram(0);
	GL_checkError();
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
