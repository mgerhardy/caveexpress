#include "TestShared.h"
#include "common/Log.h"
#include "common/System.h"

#if 0
TEST(SystemTest, testOpenURL)
{
	std::vector<std::string> args;
	ASSERT_EQ(0, System.openURL("http://127.0.0.1"));
}
#endif

TEST(SystemTest, testGetCurrentWorkingDir)
{
	ASSERT_NE("", System.getCurrentWorkingDir());
}

TEST(SystemTest, testGetCurrentUser)
{
	ASSERT_NE("", System.getCurrentUser());
}

TEST(SystemTest, testGetHomeDirectory)
{
	ASSERT_NE("", System.getHomeDirectory());
}
