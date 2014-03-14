#pragma once

#include "engine/common/Config.h"
#include "engine/common/Logger.h"
#include <SDL_platform.h>
#include <SDL_config.h>
#include <SDL_video.h>

#if SDL_VIDEO_OPENGL
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#elif SDL_VIDEO_OPENGL_ES
#include <SDL_opengles2.h>
#endif

#ifndef SDL_VIDEO_OPENGL
#ifndef SDL_VIDEO_OPENGL_ES
#error "No opengl or opengles found"
#endif
#endif

/* glsl vertex and fragment shaders and programs */
typedef GLuint (*CreateShader_t)(GLenum type);
typedef void (*DeleteShader_t)(GLuint id);
typedef void (*ShaderSource_t)(GLuint id, GLuint count, const GLchar **sources, GLuint *len);
typedef void (*CompileShader_t)(GLuint id);
typedef void (*GetShaderiv_t)(GLuint id, GLenum field, GLint *dest);
typedef void (*GetShaderInfoLog_t)(GLuint id, GLuint maxlen, GLuint *len, GLchar *dest);
typedef GLuint (*CreateProgram_t)(void);
typedef void (*DeleteProgram_t)(GLuint id);
typedef void (*AttachShader_t)(GLuint prog, GLuint shader);
typedef void (*DetachShader_t)(GLuint prog, GLuint shader);
typedef void (*LinkProgram_t)(GLuint id);
typedef void (*UseProgram_t)(GLuint id);
typedef void (*GetActiveUniforms_t)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (*GetProgramiv_t)(GLuint id, GLenum field, GLint *dest);
typedef void (*GetProgramInfoLog_t)(GLuint id, GLuint maxlen, GLuint *len, GLchar *dest);
typedef GLint (*GetUniformLocation_t)(GLuint id, const GLchar *name);
typedef void (*Uniform1i_t)(GLint location, GLint i);
typedef void (*Uniform2i_t)(GLint location, GLint i1, GLint i2);
typedef void (*Uniform3i_t)(GLint location, GLint i1, GLint i2, GLint i3);
typedef void (*Uniform4i_t)(GLint location, GLint i1, GLint i2, GLint i3, GLint i4);
typedef void (*Uniform1f_t)(GLint location, GLfloat f);
typedef void (*Uniform2f_t)(GLint location, GLfloat f1, GLfloat f2);
typedef void (*Uniform3f_t)(GLint location, GLfloat f1, GLfloat f2, GLfloat f3);
typedef void (*Uniform4f_t)(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4);
typedef void (*Uniform1fv_t)(GLint location, int count, GLfloat *f);
typedef void (*Uniform2fv_t)(GLint location, int count, GLfloat *f);
typedef void (*Uniform3fv_t)(GLint location, int count, GLfloat *f);
typedef void (*Uniform4fv_t)(GLint location, int count, GLfloat *f);
typedef void (*GetActiveAttrib_t)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef GLint (*GetAttribLocation_t)(GLuint id, const GLchar *name);
typedef void (*UniformMatrix2fv_t)(GLint location, int count, GLboolean transpose, GLfloat *v);
typedef void (*UniformMatrix3fv_t)(GLint location, int count, GLboolean transpose, GLfloat *v);
typedef void (*UniformMatrix4fv_t)(GLint location, int count, GLboolean transpose, GLfloat *v);

/* vertex attribute arrays */
typedef void (*EnableVertexAttribArray_t)(GLuint index);
typedef void (*DisableVertexAttribArray_t)(GLuint index);
typedef void (*VertexAttribPointer_t)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (*VertexAttrib4f_t)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

/* multitexture */
typedef void (*ActiveTexture_t)(GLenum texture);
typedef void (*ClientActiveTexture_t)(GLenum texture);

class GLContext {
private:
	bool _shadersSupported;
	bool _textureRectangleSupported;

protected:
	inline uintptr_t getProcAddress (const char *functionName) const
	{
		return (uintptr_t)SDL_GL_GetProcAddress(functionName);
	}

public:
	GLContext() :
			_shadersSupported(false), _textureRectangleSupported(false) {
		/* glsl */
		ctx_glCreateShader = nullptr;
		ctx_glDeleteShader = nullptr;
		ctx_glShaderSource = nullptr;
		ctx_glCompileShader = nullptr;
		ctx_glGetShaderiv = nullptr;
		ctx_glGetShaderInfoLog = nullptr;
		ctx_glCreateProgram = nullptr;
		ctx_glDeleteProgram = nullptr;
		ctx_glAttachShader = nullptr;
		ctx_glDetachShader = nullptr;
		ctx_glLinkProgram = nullptr;
		ctx_glUseProgram = nullptr;
		ctx_glGetActiveUniform = nullptr;
		ctx_glGetProgramiv = nullptr;
		ctx_glGetProgramInfoLog = nullptr;
		ctx_glGetUniformLocation = nullptr;
		ctx_glUniform1i = nullptr;
		ctx_glUniform2i = nullptr;
		ctx_glUniform3i = nullptr;
		ctx_glUniform4i = nullptr;
		ctx_glUniform1f = nullptr;
		ctx_glUniform2f = nullptr;
		ctx_glUniform3f = nullptr;
		ctx_glUniform4f = nullptr;
		ctx_glUniform1fv = nullptr;
		ctx_glUniform2fv = nullptr;
		ctx_glUniform3fv = nullptr;
		ctx_glUniform4fv = nullptr;
		ctx_glGetActiveAttrib = nullptr;
		ctx_glGetAttribLocation = nullptr;
		ctx_glUniformMatrix2fv = nullptr;
		ctx_glUniformMatrix3fv = nullptr;
		ctx_glUniformMatrix4fv = nullptr;

		/* vertex attribute arrays */
		ctx_glEnableVertexAttribArray = nullptr;
		ctx_glDisableVertexAttribArray = nullptr;
		ctx_glVertexAttribPointer = nullptr;
		ctx_glVertexAttrib4f = nullptr;

		/* multitexture */
		ctx_glActiveTexture = nullptr;
		ctx_glClientActiveTexture = nullptr;
	}

	static inline GLContext& get ()
	{
		static GLContext context;
		return context;
	}

	inline bool areShadersSupported () const {
		return false;
	}

	inline bool isMultiTextureSupported () const {
		return ctx_glActiveTexture != nullptr && ctx_glClientActiveTexture != nullptr;
	}

	inline bool isTextureRectangleSupported () const {
		return _textureRectangleSupported;
	}

	void init() {
		_shadersSupported = SDL_GL_ExtensionSupported("GL_ARB_shader_objects") &&
			SDL_GL_ExtensionSupported("GL_ARB_shading_language_100") &&
			SDL_GL_ExtensionSupported("GL_ARB_vertex_shader") &&
			SDL_GL_ExtensionSupported("GL_ARB_fragment_shader");

		_textureRectangleSupported = SDL_GL_ExtensionSupported("GL_ARB_texture_rectangle") ||
			SDL_GL_ExtensionSupported("GL_EXT_texture_rectangle");

		/* glsl vertex and fragment shaders and programs */
		if (_shadersSupported) {
			ctx_glCreateShader = (CreateShader_t)getProcAddress("glCreateShader");
			ctx_glDeleteShader = (DeleteShader_t)getProcAddress("glDeleteShader");
			ctx_glShaderSource = (ShaderSource_t)getProcAddress("glShaderSource");
			ctx_glCompileShader = (CompileShader_t)getProcAddress("glCompileShader");
			ctx_glGetShaderiv = (GetShaderiv_t)getProcAddress("glGetShaderiv");
			ctx_glGetShaderInfoLog = (GetShaderInfoLog_t)getProcAddress("glGetShaderInfoLog");
			ctx_glCreateProgram = (CreateProgram_t)getProcAddress("glCreateProgram");
			ctx_glDeleteProgram = (DeleteProgram_t)getProcAddress("glDeleteProgram");
			ctx_glAttachShader = (AttachShader_t)getProcAddress("glAttachShader");
			ctx_glDetachShader = (DetachShader_t)getProcAddress("glDetachShader");
			ctx_glLinkProgram = (LinkProgram_t)getProcAddress("glLinkProgram");
			ctx_glUseProgram = (UseProgram_t)getProcAddress("glUseProgram");
			ctx_glGetActiveUniform = (GetActiveUniforms_t)getProcAddress("glGetActiveUniform");
			ctx_glGetProgramiv = (GetProgramiv_t)getProcAddress("glGetProgramiv");
			ctx_glGetProgramInfoLog = (GetProgramInfoLog_t)getProcAddress("glGetProgramInfoLog");
			ctx_glGetUniformLocation = (GetUniformLocation_t)getProcAddress("glGetUniformLocation");
			ctx_glUniform1i = (Uniform1i_t)getProcAddress("glUniform1i");
			ctx_glUniform2i = (Uniform2i_t)getProcAddress("glUniform2i");
			ctx_glUniform3i = (Uniform3i_t)getProcAddress("glUniform3i");
			ctx_glUniform4i = (Uniform4i_t)getProcAddress("glUniform4i");
			ctx_glUniform1f = (Uniform1f_t)getProcAddress("glUniform1f");
			ctx_glUniform2f = (Uniform2f_t)getProcAddress("glUniform2f");
			ctx_glUniform3f = (Uniform3f_t)getProcAddress("glUniform3f");
			ctx_glUniform4f = (Uniform4f_t)getProcAddress("glUniform4f");
			ctx_glUniform1fv = (Uniform1fv_t)getProcAddress("glUniform1fv");
			ctx_glUniform2fv = (Uniform2fv_t)getProcAddress("glUniform2fv");
			ctx_glUniform3fv = (Uniform3fv_t)getProcAddress("glUniform3fv");
			ctx_glUniform4fv = (Uniform4fv_t)getProcAddress("glUniform4fv");
			ctx_glGetActiveAttrib = (GetActiveAttrib_t)getProcAddress("glGetActiveAttrib");
			ctx_glGetAttribLocation = (GetAttribLocation_t)getProcAddress("glGetAttribLocation");
			ctx_glUniformMatrix2fv = (UniformMatrix2fv_t)getProcAddress("glUniformMatrix2fv");
			ctx_glUniformMatrix3fv = (UniformMatrix3fv_t)getProcAddress("glUniformMatrix3fv");
			ctx_glUniformMatrix4fv = (UniformMatrix4fv_t)getProcAddress("glUniformMatrix4fv");

			/* vertex attribute arrays */
			ctx_glEnableVertexAttribArray = (EnableVertexAttribArray_t)getProcAddress("glEnableVertexAttribArray");
			ctx_glDisableVertexAttribArray = (DisableVertexAttribArray_t)getProcAddress("glDisableVertexAttribArray");
			ctx_glVertexAttribPointer = (VertexAttribPointer_t)getProcAddress("glVertexAttribPointer");
			ctx_glVertexAttrib4f = (VertexAttrib4f_t)getProcAddress("glVertexAttrib4f");
		}

		/* multitexture */
		if (SDL_GL_ExtensionSupported("GL_ARB_multitexture")) {
			ctx_glActiveTexture = (ActiveTexture_t)getProcAddress("glActiveTexture");
			ctx_glClientActiveTexture = (ClientActiveTexture_t)getProcAddress("glClientActiveTexture");
		}
	}

	/* glsl vertex and fragment shaders and programs */
	GLuint (*ctx_glCreateShader)(GLenum type);
	void (*ctx_glDeleteShader)(GLuint id);
	void (*ctx_glShaderSource)(GLuint id, GLuint count, const GLchar **sources, GLuint *len);
	void (*ctx_glCompileShader)(GLuint id);
	void (*ctx_glGetShaderiv)(GLuint id, GLenum field, GLint *dest);
	void (*ctx_glGetShaderInfoLog)(GLuint id, GLuint maxlen, GLuint *len, GLchar *dest);
	GLuint (*ctx_glCreateProgram)(void);
	void (*ctx_glDeleteProgram)(GLuint id);
	void (*ctx_glAttachShader)(GLuint prog, GLuint shader);
	void (*ctx_glDetachShader)(GLuint prog, GLuint shader);
	void (*ctx_glLinkProgram)(GLuint id);
	void (*ctx_glUseProgram)(GLuint id);
	void (*ctx_glGetProgramiv)(GLuint id, GLenum field, GLint *dest);
	void (*ctx_glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
	void (*ctx_glGetProgramInfoLog)(GLuint id, GLuint maxlen, GLuint *len, GLchar *dest);
	GLint (*ctx_glGetUniformLocation)(GLuint id, const GLchar *name);
	void (*ctx_glUniform1i)(GLint location, GLint i);
	void (*ctx_glUniform2i)(GLint location, GLint i1, GLint i2);
	void (*ctx_glUniform3i)(GLint location, GLint i1, GLint i2, GLint i3);
	void (*ctx_glUniform4i)(GLint location, GLint i1, GLint i2, GLint i3, GLint i4);
	void (*ctx_glUniform1f)(GLint location, GLfloat f);
	void (*ctx_glUniform2f)(GLint location, GLfloat f1, GLfloat f2);
	void (*ctx_glUniform3f)(GLint location, GLfloat f1, GLfloat f2, GLfloat f3);
	void (*ctx_glUniform4f)(GLint location, GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4);
	void (*ctx_glUniform1fv)(GLint location, int count, GLfloat *f);
	void (*ctx_glUniform2fv)(GLint location, int count, GLfloat *f);
	void (*ctx_glUniform3fv)(GLint location, int count, GLfloat *f);
	void (*ctx_glUniform4fv)(GLint location, int count, GLfloat *f);
	void (*ctx_glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	GLint (*ctx_glGetAttribLocation)(GLuint id, const GLchar *name);
	void (*ctx_glUniformMatrix2fv)(GLint location, int count, GLboolean transpose, GLfloat *v);
	void (*ctx_glUniformMatrix3fv)(GLint location, int count, GLboolean transpose, GLfloat *v);
	void (*ctx_glUniformMatrix4fv)(GLint location, int count, GLboolean transpose, GLfloat *v);
	void (*ctx_glVertexAttrib4f) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

	/* vertex attribute arrays */
	void (*ctx_glEnableVertexAttribArray)(GLuint index);
	void (*ctx_glDisableVertexAttribArray)(GLuint index);
	void (*ctx_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

	/* multitexture */
	void (*ctx_glActiveTexture)(GLenum texture);
	void (*ctx_glClientActiveTexture)(GLenum texture);
};

inline const char* translateError (GLenum glError)
{
#define GL_ERROR_TRANSLATE(e) case e: return #e;
	switch (glError) {
	/* openGL errors */
	GL_ERROR_TRANSLATE(GL_INVALID_ENUM)
	GL_ERROR_TRANSLATE(GL_INVALID_VALUE)
	GL_ERROR_TRANSLATE(GL_INVALID_OPERATION)
	GL_ERROR_TRANSLATE(GL_OUT_OF_MEMORY)
#ifndef SDL_VIDEO_OPENGL_ES
	GL_ERROR_TRANSLATE(GL_STACK_OVERFLOW)
	GL_ERROR_TRANSLATE(GL_STACK_UNDERFLOW)
#endif
	default:
		return "UNKNOWN";
	}
#undef GL_ERROR_TRANSLATE
}

inline int OpenGLStateHandlerCheckError (const char *file, int line, const char *function)
{
	int ret = 0;
	/* check gl errors (can return multiple errors) */
	for (;;) {
		const GLenum glError = glGetError();
		if (glError == GL_NO_ERROR)
			break;

		error(LOG_CLIENT, String::format("openGL err: %s (%d): %s %s (0x%X)",
				file, line, function, translateError(glError), glError));
		ret++;
	}
	return ret;
}

#if defined(SDL_VIDEO_OPENGL) || defined(SDL_VIDEO_OPENGL_ES)
#ifdef DEBUG
#define GL_checkError() OpenGLStateHandlerCheckError(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#else
#define GL_checkError()
#endif
#else
#define GL_checkError()
#endif
