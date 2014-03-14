#pragma once

#include "engine/client/particles/IParticleEnvironment.h"
#include "engine/common/IFrontend.h"
#include "engine/client/textures/Texture.h"
#include "engine/common/Pointers.h"
#include <string>

class Particle {
protected:
	IParticleEnvironment& _env;
	TexturePtr _texture;
	bool _active;
	uint32_t _time;
	float _deltaTime;
	float _alpha;
	uint16_t _angle;
	float _fps;
	float _tps;
	float _lastFrame;
	float _lastThink;
	float _t;
	// life time
	float _life;
	// acceleration
	vec2 _a;
	// velocity
	vec2 _v;
	// starting speed
	vec2 _s;
	// angle movement
	float _omega;

	TexturePtr loadTexture (const std::string& image) const;
	void advanceVector (const vec2& veca, const float scale, const vec2& vecb, vec2& outVector) const;
public:
	Particle(IParticleEnvironment& env);
	virtual ~Particle();

	// this is called with each update call
	virtual void run () {}
	// this can be called multiple times in a frame - depends on the tps value
	virtual void think () {}
	// this is called when the particle is spawned
	virtual void init () {}

	// returns false if the particle is going to be destroyed
	bool update (uint32_t deltaTime);
	virtual void render (IFrontend* frontend, int x = 0, int y = 0) const;

	std::string toString() const;
};

typedef SharedPtr<Particle> ParticlePtr;
