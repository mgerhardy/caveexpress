#include "WaterShader.h"
#include "engine/client/ui/UI.h"

WaterShader::WaterShader () :
		Shader()
{
}

WaterShader::~WaterShader ()
{
}

bool WaterShader::init ()
{
	if (!loadProgram("water"))
		return false;

	_displacement = UI::get().loadTexture("waterdisplacement");
	_water = UI::get().loadTexture("water");

	_initialized = _displacement->isValid() && _water->isValid();
	return _initialized;
}

bool WaterShader::use (int x, int y, int w, int h)
{
	if (!_initialized)
		return false;

	if (!activate())
		return false;

	float angle = _time * M_PI_2;
	if (angle > M_PI_2)
		angle -= M_PI_2;

	_displacement->bindTexture(0);
	_water->bindTexture(1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_checkError();
	glEnable(GL_BLEND);
	GL_checkError();
	setUniformMatrix("u_worldView", _modelViewMatrix);
	setUniformi("u_texture", 0);
	setUniformi("u_texture2", 1);
	setUniformf("timedelta", -angle);

	createAndBindAttributesArray(x, y, w, h);
	GL_checkError();

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	GL_checkError();

	deactivate();

	return true;
}

void WaterShader::createAndBindAttributesArray (int x, int y, int w, int h)
{
	float posBuffer[] = {-1, -1, 1, -1, 1, -0.3f, -1, -0.3f};
	setVertexAttribute("a_position", 2, GL_FLOAT, false, 0, posBuffer);
	float texCoordBuffer[] = {1, 1, 0, 1, 0, 0, 1, 0};
	setVertexAttribute("a_texCoord0", 2, GL_FLOAT, false, 0, texCoordBuffer);
}
