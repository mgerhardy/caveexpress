#pragma once

#include "engine/common/Config.h"
#include "engine/common/Logger.h"
#include <SDL_platform.h>
#include <SDL_assert.h>
#include <SDL_config.h>
#include <SDL_video.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

typedef unsigned int TexNum;

class GLContext {
private:
	bool _shadersSupported;
	bool _textureRectangleSupported;

public:
	GLContext() :
			_shadersSupported(false), _textureRectangleSupported(false) {
	}

	inline bool areShadersSupported () const {
		return _shadersSupported;
	}

	void init() {
		_shadersSupported = SDL_GL_ExtensionSupported("GL_ARB_shader_objects") &&
			SDL_GL_ExtensionSupported("GL_ARB_shading_language_100") &&
			SDL_GL_ExtensionSupported("GL_ARB_vertex_shader") &&
			SDL_GL_ExtensionSupported("GL_ARB_fragment_shader");
	}
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
	SDL_assert_always(ret == 0);
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
