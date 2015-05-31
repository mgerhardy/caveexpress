#pragma once

#include "TestShared.h"
#include "common/network/ProtocolMessageFactory.h"

class AbstractProtocolTest: public MapSuite {
protected:
	void testSharedMessages() const {
		testMessage(PingMessage("name", "mapName", 42, 0, 1));
		testMessage(LoadMapMessage("name", "title"));
		testMessage(ClientInitMessage("name"));
		testMessage(ErrorMessage(UNKNOWN_ENTITY, 1));
		testMessage(ChangeAnimationMessage(1, Animation::NONE));
		testMessage(MapRestartMessage(42));
		testMessage(TextMessage("message"));
		testMessage(PauseMessage(false));
		testMessage(AddEntityMessage(1, EntityType::NONE, Animation::NONE, "sprite", 0.0f, 0.0f, 0.0f, 0.0f, 0, ENTITY_ALIGN_UPPER_LEFT));
		testMessage(UpdateEntityMessage(1, 0.0f, 0.0f, 42, 1));
		std::map<std::string, std::string> settings;
		settings["foo"] = "bar";
		settings["bar"] = "foo";
		testMessage(MapSettingsMessage(settings, 1));
		testMessage(InitDoneMessage(1, 4, 3, 100));
		testMessage(RemoveEntityMessage(1, false));
		testMessage(SpawnInfoMessage(0.0f, 0.0f, EntityType::NONE));
		testMessage(RumbleMessage(1.0f, 1000));
		testMessage(BackToMainMessage("window"));
		testMessage(FinishedMapMessage("mapName", 0, 1, 2));
		testMessage(StopMovementMessage(DIRECTION_UP));
		testMessage(MovementMessage(DIRECTION_UP));
		testMessage(FingerMovementMessage(1, 2));
		testMessage(FailedMapMessage("mapName", MapFailedReason::NONE, ThemeType::NONE));
		std::vector<UpdateParticleEntity> bodies;
		UpdateParticleEntity b;
		b.x = 1.0f;
		b.y = 2.0f;
		b.angle = 0;
		b.index = 0;
		b.lifetime = 2000;
		bodies.push_back(b);
		testMessage(UpdateParticleMessage(1, bodies, 100, 2000));
		testMessage(SoundMessage(0.0f, 0.0f, SoundType::NONE));
		testMessage(UpdateHitpointsMessage(99));
		testMessage(UpdateLivesMessage(2));
		testMessage(TimeRemainingMessage(1));
		testMessage(UpdatePointsMessage(12314));
		testMessage(UpdatePackageCountMessage(254));
	}

	template<class MSG>
	void testMessage(const MSG& msg) const {
		ByteStream s;
		msg.serialize(s);
		const protocolId id = *s.getBuffer();
		IProtocolMessage* m = ProtocolMessageFactory::get().create(s);
		EXPECT_TRUE(s.empty());
		EXPECT_EQ(id, m->getId());
		delete m;
	}
};
