#pragma once

#include "Shader.h"

class WaterShader: public Shader {
private:
	TexturePtr _displacement;
	TexturePtr _water;

	void createAndBindAttributesArray (int x, int y, int w, int h);

public:
	WaterShader ();
	virtual ~WaterShader ();

	bool init ();

	bool use (int x, int y, int w, int h);
};
