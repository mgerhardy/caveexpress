#pragma once

#include "engine/common/NonCopyable.h"

#include "WaterShader.h"

class ShaderManager: public NonCopyable {
private:
	ShaderManager ();
	virtual ~ShaderManager ();

	WaterShader _waterShader;
	Shader _mainShader;
	Shader _solidShader;

public:
	static ShaderManager& get ()
	{
		static ShaderManager theInstance;
		return theInstance;
	}

	inline WaterShader& getWaterShader ()
	{
		return _waterShader;
	}

	inline Shader& getMainShader ()
	{
		return _mainShader;
	}

	inline Shader& getSolidShader ()
	{
		return _solidShader;
	}

	void init ();
	void update (uint32_t deltaTime);

	void updateProjectionMatrix (int w, int h);
};
