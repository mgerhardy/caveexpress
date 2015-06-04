#pragma once

#include "TestShared.h"
#include "network/ProtocolMessageFactory.h"

#include "network/messages/PingMessage.h"
#include "network/messages/LoadMapMessage.h"
#include "network/messages/ClientInitMessage.h"
#include "network/messages/ErrorMessage.h"
#include "network/messages/ChangeAnimationMessage.h"
#include "network/messages/MapRestartMessage.h"
#include "network/messages/TextMessage.h"
#include "network/messages/PauseMessage.h"
#include "network/messages/AddEntityMessage.h"
#include "network/messages/UpdateEntityMessage.h"
#include "network/messages/MapSettingsMessage.h"
#include "network/messages/InitDoneMessage.h"
#include "network/messages/RemoveEntityMessage.h"
#include "network/messages/SpawnInfoMessage.h"
#include "network/messages/RumbleMessage.h"
#include "network/messages/BackToMainMessage.h"
#include "network/messages/FinishedMapMessage.h"
#include "network/messages/StopMovementMessage.h"
#include "network/messages/MovementMessage.h"
#include "network/messages/FingerMovementMessage.h"
#include "network/messages/FailedMapMessage.h"
#include "network/messages/UpdateParticleMessage.h"
#include "network/messages/SoundMessage.h"
#include "network/messages/UpdateHitpointsMessage.h"
#include "network/messages/UpdateLivesMessage.h"
#include "network/messages/TimeRemainingMessage.h"
#include "network/messages/UpdatePointsMessage.h"
#include "network/messages/UpdatePackageCountMessage.h"

class AbstractProtocolTest: public AbstractTest {
protected:
	void testSharedMessages() const {
		testMessage("PingMessage", PingMessage("name", "mapName", 42, 0, 1));
		testMessage("LoadMapMessage", LoadMapMessage("name", "title"));
		testMessage("ClientInitMessage", ClientInitMessage("name"));
		testMessage("ErrorMessage", ErrorMessage(UNKNOWN_ENTITY, 1));
		testMessage("ChangeAnimationMessage", ChangeAnimationMessage(1, Animation::NONE));
		testMessage("MapRestartMessage", MapRestartMessage(42));
		testMessage("TextMessage", TextMessage("message"));
		testMessage("PauseMessage", PauseMessage(false));
		testMessage("AddEntityMessage", AddEntityMessage(1, EntityType::NONE, Animation::NONE, "sprite", 0.0f, 0.0f, 0.0f, 0.0f, 0, ENTITY_ALIGN_UPPER_LEFT));
		testMessage("UpdateEntityMessage", UpdateEntityMessage(1, 0.0f, 0.0f, 42, 1));
		std::map<std::string, std::string> settings;
		settings["foo"] = "bar";
		settings["bar"] = "foo";
		testMessage("MapSettingsMessage", MapSettingsMessage(settings, 1));
		testMessage("InitDoneMessage", InitDoneMessage(1, 4, 3, 100));
		testMessage("RemoveEntityMessage", RemoveEntityMessage(1, false));
		testMessage("SpawnInfoMessage", SpawnInfoMessage(0.0f, 0.0f, EntityType::NONE));
		testMessage("RumbleMessage", RumbleMessage(1.0f, 1000));
		testMessage("BackToMainMessage", BackToMainMessage("window"));
		testMessage("FinishedMapMessage", FinishedMapMessage("mapName", 0, 1, 2));
		testMessage("StopMovementMessage", StopMovementMessage(DIRECTION_UP));
		testMessage("MovementMessage", MovementMessage(DIRECTION_UP));
		testMessage("FingerMovementMessage", FingerMovementMessage(1, 2));
		testMessage("FailedMapMessage", FailedMapMessage("mapName", MapFailedReason::NONE, ThemeType::NONE));
		std::vector<UpdateParticleEntity> bodies;
		UpdateParticleEntity b;
		b.x = 1.0f;
		b.y = 2.0f;
		b.angle = 0;
		b.index = 0;
		b.lifetime = 2000;
		bodies.push_back(b);
		testMessage("UpdateParticleMessage", UpdateParticleMessage(1, bodies, 100, 2000));
		testMessage("SoundMessage", SoundMessage(0.0f, 0.0f, SoundType::NONE));
		testMessage("UpdateHitpointsMessage", UpdateHitpointsMessage(99));
		testMessage("UpdateLivesMessage", UpdateLivesMessage(2));
		testMessage("TimeRemainingMessage", TimeRemainingMessage(1));
		testMessage("UpdatePointsMessage", UpdatePointsMessage(12314));
		testMessage("UpdatePackageCountMessage", UpdatePackageCountMessage(254));
	}

	template<class MSG>
	void testMessage(const std::string& msgName, const MSG& msg) const {
		ByteStream s;
		msg.serialize(s);
		const protocolId id = *s.getBuffer();
		IProtocolMessage* m = ProtocolMessageFactory::get().createMsg(s);
		EXPECT_TRUE(s.empty()) << msgName;
		EXPECT_EQ(id, m->getId()) << msgName;
	}
};
