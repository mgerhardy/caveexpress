/**
 * @file
 * @brief Render maps to PNG via the game's Map::load + ClientMap::render path.
 *
 * Built twice from this file:
 *   caveexpress-map2png  (MAP2PNG_CAVEEXPRESS)
 *   cavepacker-map2png   (MAP2PNG_CAVEPACKER)
 *
 * The two game shared libraries cannot be linked into one process (SpriteType
 * and EntityType names collide in a global Enum map). Run from the repo root.
 */

#if defined(MAP2PNG_CAVEEXPRESS) == defined(MAP2PNG_CAVEPACKER)
#error "Build exactly one of MAP2PNG_CAVEEXPRESS or MAP2PNG_CAVEPACKER"
#endif

#include "common/Application.h"
#include "common/ConfigManager.h"
#include "common/Singleton.h"
#include "common/EventHandler.h"
#include "common/FileSystem.h"
#include "common/IConsole.h"
#include "common/Shared.h"
#include "common/SpriteDefinition.h"
#include "common/String.h"
#include "common/TextureDefinition.h"
#include "client/ClientMap.h"
#include "client/network/ChangeAnimationHandler.h"
#include "client/network/MapSettingsHandler.h"
#include "client/network/SoundHandler.h"
#include "client/network/UpdateEntityHandler.h"
#include "gfx/SDLFrontend.h"
#include "network/INetwork.h"
#include "network/IProtocolHandler.h"
#include "network/IServerCallback.h"
#include "network/ProtocolHandlerRegistry.h"
#include "network/messages/LoadMapMessage.h"
#include "service/ServiceProvider.h"
#include "ui/UI.h"
#ifdef MAP2PNG_CAVEEXPRESS
#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/client/ClientEntityFactories.h"
#include "caveexpress/client/ClientMapHandlers.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/CaveExpressConfig.h"
#include "caveexpress/shared/network/ProtocolMessageFactories.h"
#endif
#ifdef MAP2PNG_CAVEPACKER
#include "cavepacker/client/CavePackerClientMap.h"
#include "cavepacker/client/ClientEntityFactories.h"
#include "cavepacker/client/ClientMapHandlers.h"
#include "cavepacker/server/entities/Player.h"
#include "cavepacker/server/map/Map.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#endif

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace {

#ifdef MAP2PNG_CAVEEXPRESS
const char* const kToolName = "caveexpress-map2png";
const char* const kGameName = "caveexpress";
const char* const kMapExt = ".lua";
#else
const char* const kToolName = "cavepacker-map2png";
const char* const kGameName = "cavepacker";
const char* const kMapExt = ".sok";
#endif

class NullConsole: public IConsole {
public:
	void init (IFrontend*) override {}
	void logInfo (const std::string&) override {}
	void logError (const std::string&) override {}
	void logDebug (const std::string&) override {}
	void render () override {}
	void update (uint32_t) override {}
};

class ClientLoadMapHandler: public ClientProtocolHandler<LoadMapMessage> {
	ClientMap& _map;
public:
	explicit ClientLoadMapHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const LoadMapMessage* msg) override
	{
		_map.load(msg->getName(), msg->getTitle());
	}
};

struct Options {
	std::string textureSize = "small";
	int extraScale = 1;
	std::string out;
	bool all = false;
	std::vector<std::string> maps;
};

void usage ()
{
	std::fprintf(stderr,
			"Usage: %s [options] <map> [<map> ...]\n"
			"       %s [options] --all\n"
			"\n"
			"Render %s maps to PNG via the game client renderer. Run from the repo root.\n"
			"\n"
			"Options:\n"
			"  --size small|big                     Texture atlas size (default: small)\n"
			"  --scale N                            Extra zoom (default: 1)\n"
			"  --out PATH                           Output file, or directory with --all / multiple maps\n"
			"  --all                                Render every map\n"
			"  -h, --help                           Show this help\n"
			"\n"
			"Maps can be ids or files under base/%s/maps/.\n",
			kToolName, kToolName, kGameName, kGameName);
}

bool parseArgs (int argc, char** argv, Options& opt)
{
	for (int i = 1; i < argc; ++i) {
		const std::string a = argv[i];
		if (a == "-h" || a == "--help") {
			usage();
			return false;
		}
		if (a == "--all") {
			opt.all = true;
			continue;
		}
		if (a == "--size" && i + 1 < argc) {
			opt.textureSize = argv[++i];
			if (opt.textureSize != "small" && opt.textureSize != "big") {
				std::fprintf(stderr, "--size must be small or big\n");
				return false;
			}
			continue;
		}
		if (a == "--scale" && i + 1 < argc) {
			opt.extraScale = std::max(1, atoi(argv[++i]));
			continue;
		}
		if (a == "--out" && i + 1 < argc) {
			opt.out = argv[++i];
			continue;
		}
		if (string::startsWith(a, "-")) {
			std::fprintf(stderr, "unknown option '%s'\n", a.c_str());
			usage();
			return false;
		}
		opt.maps.push_back(a);
	}
	if (!opt.all && opt.maps.empty()) {
		usage();
		return false;
	}
	return true;
}

std::string mapIdFromArg (const std::string& arg)
{
	std::string id = arg;
	const std::string::size_type slash = id.find_last_of("/\\");
	if (slash != std::string::npos)
		id = id.substr(slash + 1);
	if (string::endsWith(id, ".lua"))
		id = id.substr(0, id.size() - 4);
	else if (string::endsWith(id, ".sok"))
		id = id.substr(0, id.size() - 4);
	return id;
}

void ensureParentDir (const std::string& path)
{
	for (std::string::size_type i = 1; i < path.size(); ++i) {
		if (path[i] != '/' && path[i] != '\\')
			continue;
		const std::string dir = path.substr(0, i);
#ifdef _WIN32
		_mkdir(dir.c_str());
#else
		mkdir(dir.c_str(), 0755);
#endif
	}
}

std::string outputPath (const Options& opt, const std::string& mapId, bool many)
{
	if (opt.out.empty())
		return mapId + ".png";
	if (!many && (string::endsWith(opt.out, ".png") || string::endsWith(opt.out, ".PNG")))
		return opt.out;
	std::string dir = opt.out;
	if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
		dir += '/';
	return dir + mapId + ".png";
}

struct Renderer {
	Options opt;
	EventHandler events;
	ServiceProvider services;
	SDLFrontend frontend;
	IServerCallback serverCb;
#ifdef MAP2PNG_CAVEEXPRESS
	caveexpress::Map serverMap;
	std::unique_ptr<caveexpress::CaveExpressClientMap> map;
#else
	cavepacker::Map serverMap;
	std::unique_ptr<cavepacker::CavePackerClientMap> map;
#endif
	int extraScale = 1;
	int tileSize = 64;
	bool frontendReady = false;
	bool assetsLoaded = false;
	bool networkOpen = false;

	explicit Renderer (const Options& options) :
			opt(options), frontend(std::make_shared<NullConsole>()), extraScale(options.extraScale)
	{
		services.initForTool(&events);
		Config.getConfigVar("maxzoom", "1.2")->setValue(string::toString(std::max(2, extraScale)));
		Config.getConfigVar("minzoom", "0.5")->setValue("0.25");
	}

	~Renderer ()
	{
		closeNetwork();
		map.reset();
		serverMap.shutdown();
		UI::get().shutdownAssets();
	}

	int scaledTile () const
	{
		return tileSize * extraScale;
	}

	void closeNetwork ()
	{
		INetwork& net = services.getNetwork();
		net.closeClient();
		net.closeServer();
		networkOpen = false;
	}

	void recreateMap ()
	{
		map.reset();
#ifdef MAP2PNG_CAVEEXPRESS
		map.reset(new caveexpress::CaveExpressClientMap(0, 0, frontend.getWidth(), frontend.getHeight(), &frontend,
				services, tileSize));
		caveexpress::registerClientMapHandlers(*map);
#else
		map.reset(new cavepacker::CavePackerClientMap(0, 0, frontend.getWidth(), frontend.getHeight(), &frontend,
				services, tileSize));
		cavepacker::registerClientMapHandlers(*map);
#endif
		ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
		r.unregisterClientHandler(::protocol::PROTO_LOADMAP);
		r.registerClientHandler(::protocol::PROTO_LOADMAP, new ClientLoadMapHandler(*map));
		r.unregisterClientHandler(::protocol::PROTO_MAPSETTINGS);
		r.registerClientHandler(::protocol::PROTO_MAPSETTINGS, new MapSettingsHandler(*map));
		r.unregisterClientHandler(::protocol::PROTO_SOUND);
		r.registerClientHandler(::protocol::PROTO_SOUND, new SoundHandler());
		r.unregisterClientHandler(::protocol::PROTO_UPDATEENTITY);
		r.registerClientHandler(::protocol::PROTO_UPDATEENTITY, new UpdateEntityHandler(*map));
		r.unregisterClientHandler(::protocol::PROTO_CHANGEANIMATION);
		r.registerClientHandler(::protocol::PROTO_CHANGEANIMATION, new ChangeAnimationHandler(*map));
	}

	bool initGame ()
	{
		map.reset();
		if (frontendReady)
			UI::get().shutdownAssets();
		assetsLoaded = false;
		Singleton<Application>::getInstance().setName(kGameName);
		FS.setGame(kGameName);

		services.initTextureDefinition(&frontend, opt.textureSize);
		if (services.getTextureDefinition().getSize() == 0) {
			std::fprintf(stderr, "no textures for %s (%s)\n", kGameName, opt.textureSize.c_str());
			return false;
		}
#ifdef MAP2PNG_CAVEPACKER
		if (cavepacker::SpriteTypes::SOLID.name.empty() || cavepacker::SpriteTypes::GROUND.name.empty()
				|| cavepacker::SpriteTypes::TARGET.name.empty() || cavepacker::SpriteTypes::PACKAGE.name.empty()) {
			std::fprintf(stderr, "cavepacker sprite types failed to register\n");
			return false;
		}
#endif
		SpriteDefinition::get().init(services.getTextureDefinition());
#ifdef MAP2PNG_CAVEEXPRESS
		caveexpress::registerCaveExpressConfigVars();
		caveexpress::registerCaveExpressProtocolMessages();
		caveexpress::registerClientEntityFactories();
		caveexpress::GameEventHandler::get().init(services);
#else
		cavepacker::registerClientEntityFactories();
#endif
		serverMap.init(&frontend, services);

		const TextureDefinition& texDef = services.getTextureDefinition();
		if (texDef.exists("tile-reference")) {
			const int w = texDef.getTextureDef("tile-reference").trim.untrimmedWidth;
			tileSize = w > 0 ? w : 64;
		} else {
			tileSize = opt.textureSize == "big" ? 128 : 64;
		}
		return true;
	}

	bool prepareCanvas (int pixelW, int pixelH)
	{
		closeNetwork();
		if (!frontendReady) {
			if (frontend.initOffscreen(pixelW, pixelH) != 0) {
				std::fprintf(stderr, "failed to init offscreen renderer\n");
				return false;
			}
			frontendReady = true;
		} else if (pixelW > frontend.getWidth() || pixelH > frontend.getHeight()) {
			map.reset();
			UI::get().shutdownAssets();
			assetsLoaded = false;
			frontend.ensureOffscreenSize(pixelW, pixelH);
		}
		if (!assetsLoaded) {
			UI::get().initAssets(frontend, services);
			assetsLoaded = true;
		}
		recreateMap();
		map->setSize(pixelW, pixelH);
		map->setZoom(static_cast<float>(extraScale));
		return true;
	}

	bool openLoopback ()
	{
		INetwork& net = services.getNetwork();
		closeNetwork();
		if (!net.openServer(Config.getPort(), &serverCb)) {
			std::fprintf(stderr, "failed to open loopback server\n");
			return false;
		}
		if (!net.openClient("localhost", Config.getPort(), map.get())) {
			std::fprintf(stderr, "failed to open loopback client\n");
			return false;
		}
		networkOpen = true;
		return true;
	}

	void pumpNetwork ()
	{
		services.getNetwork().update(0);
	}

	bool capture (const std::string& pngPath, int pixelW, int pixelH)
	{
		map->setSize(pixelW, pixelH);
		map->setZoom(static_cast<float>(extraScale));
		map->start();
		map->update(0);
		frontend.renderBegin();
		map->render();
		frontend.renderEnd(false);
		ensureParentDir(pngPath);
		if (!frontend.savePng(pngPath, pixelW, pixelH)) {
			std::fprintf(stderr, "failed to write %s\n", pngPath.c_str());
			return false;
		}
		std::printf("wrote %s (%ix%i)\n", pngPath.c_str(), pixelW, pixelH);
		return true;
	}

	bool renderMap (const std::string& mapId, const std::string& pngPath)
	{
		closeNetwork();
		serverMap.shutdown();
		INetwork& net = services.getNetwork();
		if (!net.openServer(Config.getPort(), &serverCb)) {
			std::fprintf(stderr, "failed to open loopback server\n");
			return false;
		}
		if (!serverMap.load(mapId)) {
			std::fprintf(stderr, "failed to load %s map '%s'\n", kGameName, mapId.c_str());
			return false;
		}
		const int mapW = serverMap.getMapWidth();
		const int mapH = serverMap.getMapHeight();
		if (mapW < 1 || mapH < 1) {
			std::fprintf(stderr, "invalid dimensions for '%s'\n", mapId.c_str());
			return false;
		}
		const int pixelW = mapW * scaledTile();
		const int pixelH = mapH * scaledTile();
		serverMap.shutdown();
		closeNetwork();

		if (!prepareCanvas(pixelW, pixelH))
			return false;
		if (!openLoopback())
			return false;

		if (!serverMap.load(mapId)) {
			std::fprintf(stderr, "failed to reload %s map '%s'\n", kGameName, mapId.c_str());
			return false;
		}
		pumpNetwork();

#ifdef MAP2PNG_CAVEEXPRESS
		caveexpress::Player* player = new caveexpress::Player(serverMap, 1);
		player->setLives(3);
#else
		cavepacker::Player* player = new cavepacker::Player(serverMap, 1);
#endif
		if (!serverMap.initPlayer(player)) {
			delete player;
			std::fprintf(stderr, "failed to init player for '%s'\n", mapId.c_str());
			return false;
		}
		serverMap.startMap();
		// Emitter-spawned entities sit in _entitiesToAdd until visitEntities
		// flushes them; they become visible on the following visit. Do not
		// Map::update here: that steps physics and CaveExpress Player::update
		// marks a 0-hp or landing player as crashed.
		serverMap.visitEntities(&serverMap);
		serverMap.visitEntities(&serverMap);
		pumpNetwork();
		return capture(pngPath, pixelW, pixelH);
	}
};

std::vector<std::string> listMaps ()
{
	std::vector<std::string> ids;
	const DirectoryEntries entries = FS.listDirectory(FS.getMapsDir());
	for (const std::string& entry : entries) {
		if (!string::endsWith(entry, kMapExt))
			continue;
		ids.push_back(entry.substr(0, entry.size() - std::strlen(kMapExt)));
	}
	std::sort(ids.begin(), ids.end());
	return ids;
}

}

int main (int argc, char** argv)
{
	Options opt;
	if (!parseArgs(argc, argv, opt))
		return opt.maps.empty() && !opt.all ? EXIT_FAILURE : EXIT_SUCCESS;

	Application& app = Singleton<Application>::getInstance();
	app.setOrganisation("caveproductions");
	app.setNonInteractive(true);
	app.setName(kGameName);

	char* configArgv[] = { argv[0] };
	Config.init(nullptr, 1, configArgv);

	Renderer renderer(opt);
	if (!renderer.initGame())
		return EXIT_FAILURE;

	const std::vector<std::string> maps = opt.all ? listMaps() : opt.maps;
	const bool many = maps.size() > 1 || opt.all;
	int ok = 0;
	for (const std::string& arg : maps) {
		const std::string id = mapIdFromArg(arg);
		if (renderer.renderMap(id, outputPath(opt, id, many)))
			++ok;
	}
	return ok > 0 && ok == static_cast<int>(maps.size()) ? EXIT_SUCCESS : EXIT_FAILURE;
}
