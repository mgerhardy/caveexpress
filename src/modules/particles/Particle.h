#pragma once

#include "IParticleEnvironment.h"
#include "common/IFrontend.h"
#include "textures/Texture.h"
#include <memory>
#include <string>

class Particle {
protected:
	IParticleEnvironment& _env;
	TexturePtr _texture;
	bool _active;
	uint32_t _time;
	float _deltaTime;
	float _alpha;  // transparency
	
	float _fps;
	float _tps;
	float _lastFrame;
	float _lastThink;
	float _t;
	// life time
	float _life;
	
	// movement  ----
	// acceleration
	vec2 _a;
	// velocity
	vec2 _v;
	float _omega; // rotation velocity
	// position
	vec2 _pos;
	float _angle;  // rotation in degrees

	vec2 _scale;
	//  const wind from map
	float _wind;
	int _waterSurface;

	TexturePtr loadTexture (const std::string& image) const;
	void advanceVector (const vec2& veca, const float scale, const vec2& vecb, vec2& outVector) const;
public:
	explicit Particle(IParticleEnvironment& env);
	virtual ~Particle();

	// this is called when the particle is spawned
	virtual void init ();
	virtual void newPos ();
	// this is called with each update call
	virtual void run ();
	virtual void random () {}

	// this can be called multiple times in a frame - depends on the tps value
	virtual void think () {}

	virtual void render (IFrontend* frontend, int x = 0, int y = 0, float zoom = 1.0f) const;

	// returns false if the particle is going to be destroyed
	bool update (uint32_t deltaTime);
};

typedef std::shared_ptr<Particle> ParticlePtr;
