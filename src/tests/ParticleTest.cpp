#include "ParticleTest.h"
#include "engine/client/particles/ParticleSystem.h"
#include "engine/client/particles/Bubble.h"
#include "engine/client/particles/Snow.h"

class ParticleTest: public MapSuite {
};

class TestParticleEnv: public IParticleEnvironment {
	int getWaterSurface () const override { return getWaterGround() - getPixelHeight() / 10; }
	int getWaterGround () const override { return getPixelHeight(); }
	int getWaterWidth () const override { return getPixelWidth(); }
	int getPixelWidth () const override { return 1920; }
	int getPixelHeight () const override { return 1080; }
	TexturePtr loadTexture (const std::string& name) const override { return TexturePtr(); }
};

TEST_F(ParticleTest, testParticleSystem) {
	ParticleSystem system(10000);
	TestParticleEnv env;
	const int bubbles = 10000;
	for (int i = 0; i < bubbles; ++i) {
		system.spawn(ParticlePtr(new Bubble(env)));
	}

	ASSERT_EQ(bubbles, system.getParticleAmount());
	ASSERT_TRUE(system.hasParticles());
	system.update(1);
	system.render(&_testFrontend, 0, 0, 1.0f);
	system.clear();
	ASSERT_FALSE(system.hasParticles());
}
