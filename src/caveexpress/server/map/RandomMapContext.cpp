#include "RandomMapContext.h"
#include "RandomMapGenerator.h"
#include "common/MapSettings.h"
#include "common/String.h"
#include "common/Log.h"
#include <algorithm>
#include <cstdlib>

namespace caveexpress {

namespace {
float defaultRandomWaterHeight (unsigned int height)
{
	return std::min(2.5f, std::max(1.0f, static_cast<float>(height) * 0.18f));
}
}

RandomMapContext::RandomMapContext (const std::string& name, const ThemeType& theme,
		unsigned int width, unsigned int height) :
		CaveExpressMapContext(name), _caves(3), _mapWidth(width), _mapHeight(height),
		_waterHeight(defaultRandomWaterHeight(height)), _seedOverride(0)
{
	_theme = &theme;
	Log::info(LOG_GAMEIMPL, "random map size: %i:%i theme=%s", width, height, theme.name.c_str());
	_settings[msn::WIDTH] = string::toString(width);
	_settings[msn::HEIGHT] = string::toString(height);
	_settings[msn::THEME] = theme.name;

	setFlyingNPC(false);
	setWind(0.0f);
	setWaterParameters(_waterHeight, 0.1f, 10000L, 10000L);
	_settings[msn::GRAVITY] = string::toString(msdv::GRAVITY);
}

void RandomMapContext::setSettings (const IMap::SettingsMap& settings)
{
	for (IMap::SettingsMapConstIter i = settings.begin(); i != settings.end(); ++i)
		_settings[i->first] = i->second;
	const auto seedIt = _settings.find("seed");
	if (seedIt != _settings.end())
		_seedOverride = static_cast<unsigned int>(string::toInt(seedIt->second));
	const auto waterIt = _settings.find(msn::WATER_HEIGHT);
	if (waterIt != _settings.end())
		_waterHeight = string::toFloat(waterIt->second);
}

void RandomMapContext::setWind (float wind)
{
	_settings[msn::WIND] = string::toString(wind);
}

void RandomMapContext::setCaves (unsigned int caves)
{
	_caves = std::max(1u, caves);
}

void RandomMapContext::setFlyingNPC (bool flyingNPC)
{
	_settings[msn::FLYING_NPC] = string::toString(flyingNPC);
}

void RandomMapContext::setWaterParameters (float waterHeight, float waterChangeSpeed,
		uint32_t waterRisingDelay, uint32_t waterFallingDelay)
{
	_waterHeight = waterHeight;
	_settings[msn::WATER_HEIGHT] = string::toString(waterHeight);
	_settings[msn::WATER_CHANGE] = string::toString(waterChangeSpeed);
	_settings[msn::WATER_RISING_DELAY] = string::toString(waterRisingDelay);
	_settings[msn::WATER_FALLING_DELAY] = string::toString(waterFallingDelay);
}

void RandomMapContext::setSeed (unsigned int seed)
{
	_seedOverride = seed;
	_settings["seed"] = string::toString(seed);
}

bool RandomMapContext::load (bool /*skipErrors*/)
{
	resetTiles();
	CaveExpressMapContext::setCaveTileDefinitions({});

	if (_mapWidth == 0 || _mapHeight == 0) {
		Log::error(LOG_GAMEIMPL, "no width or height set for the random map");
		return false;
	}

	RandomMapRules rules = RandomMapRules::loadFromLua();
	if (_caves > 0)
		rules.caveTarget = static_cast<int>(_caves);

	RandomMapGenerator generator(*_theme, _mapWidth, _mapHeight, rules, _waterHeight);
	unsigned int seed = _seedOverride;
	if (seed == 0) {
		const auto seedIt = _settings.find("seed");
		if (seedIt != _settings.end())
			seed = static_cast<unsigned int>(string::toInt(seedIt->second));
	}
	if (seed == 0)
		seed = static_cast<unsigned int>(rand()) | 1u;

	Log::info(LOG_GAMEIMPL, "mapgen theme=%s size=%ux%u seed=%u caves=%i water=%.2f",
			_theme->name.c_str(), _mapWidth, _mapHeight, seed, rules.caveTarget, _waterHeight);
	RandomMapGenerator::Result result = generator.generate(seed);
	if (!result.success) {
		const std::string& reason = result.failureReason.empty() ? "unknown failure" : result.failureReason;
		Log::error(LOG_GAMEIMPL, "mapgen failed: %s", reason.c_str());
		return false;
	}

	setMapTileDefinitions(result.tiles);
	setEmitterDefinitions(result.emitters);
	setStartPositions(result.startPositions);
	CaveExpressMapContext::setCaveTileDefinitions(result.caves);
	setTitle(result.title.empty() ? _name : result.title);

	for (IMap::SettingsMapConstIter i = result.settings.begin(); i != result.settings.end(); ++i) {
		if (_settings.find(i->first) == _settings.end() || i->first == msn::THEME || i->first == msn::WATER_HEIGHT
				|| i->first == msn::WIDTH || i->first == msn::HEIGHT || i->first == "seed"
				|| i->first == msn::PACKAGE_TRANSFER_COUNT || i->first == msn::NPC_TRANSFER_COUNT)
			_settings[i->first] = i->second;
	}
	_settings["seed"] = string::toString(seed);
	const auto waterIt = _settings.find(msn::WATER_HEIGHT);
	if (waterIt != _settings.end())
		_waterHeight = string::toFloat(waterIt->second);

	Log::info(LOG_GAMEIMPL, "mapgen ready: seed=%u water=%.2f tiles=%i caves=%i emitters=%i starts=%i",
			seed, _waterHeight, (int)result.tiles.size(), (int)result.caves.size(),
			(int)result.emitters.size(), (int)result.startPositions.size());
	return true;
}

}
