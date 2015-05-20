#include "ShaderManager.h"
#include "engine/common/Logger.h"

ShaderManager::ShaderManager ()
{
	struct parameters {
		const char *desc;
		GLenum val;
	} VARS[] = {
			{ "Maximum number of vertex attributes", GL_MAX_VERTEX_ATTRIBS },
#ifdef GL_MAX_VARYING_FLOATS
			{ "Maximum number of varying floats", GL_MAX_VARYING_FLOATS },
#endif
			{ "Maximum number of texture units usable in a vertex shader", GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS },
			{ "Maximum number of texture units usable in a fragment shader", GL_MAX_TEXTURE_IMAGE_UNITS },
			{ nullptr, 0 }
	};

	for (struct parameters* p = VARS; p->desc; ++p) {
		int val = 0;
		glGetIntegerv(p->val, &val);
		info(LOG_CLIENT, String::format("%s: %i", p->desc, val));
	}
}

ShaderManager::~ShaderManager ()
{
}

void ShaderManager::updateProjectionMatrix (int w, int h)
{
	const glm::mat4 projectionMatrix = glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -5.0f, 5.0f);
	_waterShader.setProjectionMatrix(projectionMatrix);
	_mainShader.setProjectionMatrix(projectionMatrix);
	_solidShader.setProjectionMatrix(projectionMatrix);
}

void ShaderManager::init ()
{
	if (!_waterShader.init())
		error(LOG_CLIENT, "failed to initialize the water shader");
	else
		info(LOG_CLIENT, "initialize the water shader");
	if (!_mainShader.loadProgram("main"))
		error(LOG_CLIENT, "failed to initialize the main shader");
	else
		info(LOG_CLIENT, "initialize the main shader");
	if (!_solidShader.loadProgram("solid"))
		error(LOG_CLIENT, "failed to initialize the solid shader");
	else
		info(LOG_CLIENT, "initialize the solid shader");
}

void ShaderManager::update (uint32_t deltaTime)
{
	_waterShader.update(deltaTime);
	_mainShader.update(deltaTime);
	_solidShader.update(deltaTime);
}
