#include "TestShared.h"
#include <stdlib.h>
#include "common/Log.h"
#include "common/FileSystem.h"
#include "common/ConfigManager.h"
#include "common/Application.h"
#include "common/System.h"
#include <SDL.h>
#include <SDL_main.h>

struct TestConfiguration {
	bool verbose;
};

static TestConfiguration config;

class TestConsole: public IConsole {
	void init (IFrontend *frontend) override
	{
	}

	void logInfo (const std::string& string) override
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void logError (const std::string& string) override
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void logDebug (const std::string& string) override
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void render () override
	{
	}

	void update (uint32_t deltaTime) override
	{
	}
};

class LocalEnv: public ::testing::Environment {
public:
	virtual ~LocalEnv() {
	}
	virtual void SetUp() override {
	}
	virtual void TearDown() override {
	}
};

static void printUsage ()
{
	std::cout << "Usage" << std::endl;
	std::cout << "-v  --verbose        - more output than you can read" << std::endl;
	std::cout << "-h  --help           - show this help screen" << std::endl;
}

static void setDefaultOptions ()
{
	config.verbose = false;
}

static void parseCommandline (int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		const std::string parameter = argv[i];
		if (parameter == "-v" || parameter == "--verbose") {
			config.verbose = true;
			//Config.getConfigVar("debug")->setValue("true");
		} else if (parameter == "-h" || parameter == "--help") {
			printUsage();
			exit(EXIT_SUCCESS);
		}
	}
}

static TestConsole console;

extern "C" int main (int argc, char *argv[])
{
	::testing::AddGlobalTestEnvironment(new LocalEnv);
	//::testing::GTEST_FLAG(throw_on_failure) = true;
	::testing::InitGoogleTest(&argc, argv);

	setDefaultOptions();
	parseCommandline(argc, argv);

	Application& app = Singleton<Application>::getInstance();
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	app.setOrganisation("caveproductions");
	app.setName(game->getName());
	Log::get().addConsole(&console);
	Config.init(nullptr, argc, argv);
	return RUN_ALL_TESTS();
}
