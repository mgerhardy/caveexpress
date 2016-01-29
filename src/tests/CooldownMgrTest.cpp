#include "TestShared.h"
#include "cooldown/CooldownMgr.h"

TEST(CooldownMgr, testTriggerCooldown) {
	CooldownMgr mgr;
	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown couldn't get triggered";
}

TEST(CooldownMgr, testCancelCooldown) {
	CooldownMgr mgr;
	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown couldn't get triggered";
	ASSERT_TRUE(mgr.cancelCooldown(CooldownType::LOGOUT)) << "Failed to cancel the logout cooldown";
}

TEST(CooldownMgr, testExpireCooldown) {
	CooldownMgr mgr;
	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown couldn't get triggered";
	ASSERT_TRUE(mgr.isCooldown(CooldownType::LOGOUT, 0));
	mgr.update(0);
	ASSERT_TRUE(mgr.isCooldown(CooldownType::LOGOUT, 0));
	ASSERT_FALSE(mgr.isCooldown(CooldownType::LOGOUT, mgr.defaultDuration(CooldownType::LOGOUT)));
	mgr.update(mgr.defaultDuration(CooldownType::LOGOUT));
	ASSERT_FALSE(mgr.isCooldown(CooldownType::LOGOUT, 0));
	ASSERT_TRUE(mgr.resetCooldown(CooldownType::LOGOUT)) << "Failed to reset the logout cooldown";
}

TEST(CooldownMgr, testMultipleCooldown) {
	CooldownMgr mgr;

	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown couldn't get triggered";
	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::INCREASE, 0)) << "Increase cooldown couldn't get triggered";
	ASSERT_TRUE(mgr.isCooldown(CooldownType::LOGOUT, 0));
	ASSERT_TRUE(mgr.isCooldown(CooldownType::INCREASE, 0));
	mgr.update(0);

	const unsigned long logoutDuration = mgr.defaultDuration(CooldownType::LOGOUT);
	const unsigned long increaseDuration = mgr.defaultDuration(CooldownType::INCREASE);

	if (logoutDuration > increaseDuration) {
		mgr.update(increaseDuration);
		ASSERT_TRUE(mgr.isCooldown(CooldownType::LOGOUT, 0));
		ASSERT_FALSE(mgr.isCooldown(CooldownType::INCREASE, 0));
	} else {
		mgr.update(logoutDuration);
		ASSERT_TRUE(mgr.isCooldown(CooldownType::INCREASE, 0));
		ASSERT_FALSE(mgr.isCooldown(CooldownType::LOGOUT, 0));
	}
}

TEST(CooldownMgr, testTriggerCooldownTwice) {
	CooldownMgr mgr;
	ASSERT_EQ(CooldownTriggerState::SUCCESS, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown couldn't get triggered";
	ASSERT_EQ(CooldownTriggerState::ALREADY_RUNNING, mgr.triggerCooldown(CooldownType::LOGOUT, 0)) << "Logout cooldown was triggered twice";
}
