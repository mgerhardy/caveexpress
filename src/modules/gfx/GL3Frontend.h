#pragma once

#include "AbstractGLFrontend.h"
#include "gfx/Shader.h"

class GL3Frontend: public AbstractGLFrontend {
private:
	using Super = AbstractGLFrontend;
protected:
	GLuint _vao;
	GLuint _vbo;

	int _waterNoiseW = 1;
	int _waterNoiseH = 1;
	TexNum _waterNoise;
	Shader _shader;
	Shader _waterShader;

	void renderBatchesWithShader (Shader& shader);

	void newFrameImGui() override;
	void shutdownImGui() override;
	void renderImGui() override;
public:
	explicit GL3Frontend (std::shared_ptr<IConsole> console);
	virtual ~GL3Frontend ();

	bool renderWaterPlane (int x, int y, int w, int h, const Color& fillColor, const Color& lineColor, const vec2 &offsets) override;
	void initRenderer () override;
	void renderBatches () override;
};
