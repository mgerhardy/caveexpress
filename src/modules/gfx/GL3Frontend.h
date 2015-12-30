#pragma once

#include "AbstractGLFrontend.h"
#include "gfx/Shader.h"

class GL3Frontend: public AbstractGLFrontend {
protected:
	GLuint _vao;
	GLuint _vbo;

	TexNum _waterNoise;
	Shader _shader;
	Shader _waterShader;

	void renderBatchesWithShader (Shader& shader);
public:
	explicit GL3Frontend (std::shared_ptr<IConsole> console);
	virtual ~GL3Frontend ();

	bool renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& waterLineColor) override;
	void initRenderer () override;
	void renderBatches () override;
};
