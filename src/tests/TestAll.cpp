#include "TestShared.h"
#include <stdlib.h>
#include "common/Log.h"
#include "common/FileSystem.h"
#include "common/ConfigManager.h"
#include "common/Application.h"
#include "common/System.h"

struct TestConfiguration {
	bool verbose;
};

static TestConfiguration config;

class TestConsole: public IConsole {
	void init (IFrontend *frontend)
	{
	}

	void logInfo (const std::string& string)
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void logError (const std::string& string)
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void logDebug (const std::string& string)
	{
		if (!config.verbose)
			return;
		SDL_Log("%s\n", string.c_str());
	}

	void render ()
	{
	}

	void update (uint32_t deltaTime)
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

extern "C" int main (int argc, char **argv)
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
	Config.get().init(nullptr, argc, argv);
	return RUN_ALL_TESTS();
}
