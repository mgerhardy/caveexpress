#include "common/SpriteDefinition.h"
#include "common/LUALibrary.h"
#include "common/System.h"
#include "common/Log.h"
#include "common/ExecutionTime.h"
#include "common/TextureDefinition.h"

SpriteDefinition::SpriteDefinition ()
{
}

void SpriteDefinition::init (const TextureDefinition& textureDefinition)
{
	ExecutionTime e("Sprites loading");
	Log::debug(LOG_COMMON, "Sprites loading");
	LUA lua;

	if (!lua.load("sprites.lua")) {
		System.exit("could not load sprites", 1);
		return;
	}

	LUA_checkStack2(lua.getState());
	if (!lua.getGlobalKeyValue("sprites")) {
		Log::error(LOG_COMMON, "spritedef: Could not find the global sprites map");
		return;
	}

	while (lua.getNextKeyValue()) {
		LUA_checkStack2(lua.getState());
		const std::string id = lua.getKey();
		if (id.empty()) {
			Log::error(LOG_COMMON, "spritedef: no key found in definition: %s", lua.getStackDump().c_str());
			lua.pop();
			continue;
		}
		Log::debug(LOG_COMMON, "id: %s", id.c_str());

		SpriteDefMapConstIter findIter = _spriteDefs.find(id);
		if (findIter != _spriteDefs.end()) {
			Log::error(LOG_COMMON, "sprite def already defined: %s", id.c_str());
			lua.pop();
			continue;
		}

		const std::string& typeStr = lua.getValueStringFromTable("type");
		Log::debug(LOG_COMMON, "id: %s, type %s", id.c_str(), typeStr.c_str());
		const SpriteType& type = SpriteType::getByName(typeStr);
		if (!type && !typeStr.empty()) {
			Log::error(LOG_COMMON, "invalid sprite type given: %s", typeStr.c_str());
		}
		const ThemeType& theme = ThemeType::getByName(lua.getValueStringFromTable("theme"));
		SpriteDef *def = new SpriteDef(id, type, theme);

		def->fps = lua.getValueIntegerFromTable("fps", 20);
		def->redirect = lua.getValueStringFromTable("redirect");
		def->width = lua.getValueFloatFromTable("width", 1.0f);
		def->height = lua.getValueFloatFromTable("height", 1.0f);
		def->angle = lua.getValueIntegerFromTable("angle", 0);
		def->rotateable = lua.getValueIntegerFromTable("rotateable", 0);
		def->friction = lua.getValueFloatFromTable("friction", 0.2f);
		def->restitution = lua.getValueFloatFromTable("restitution", 0.0f);

		// push the frames table
		const int layers = lua.getTable("frames");
		Log::debug(LOG_COMMON, "id: %s => %i frames", id.c_str(), layers);
		if (layers > MAX_LAYERS) {
			Log::error(LOG_COMMON, "invalid sprite layer amount given for %s: %i", id.c_str(), layers);
		}

		if (layers > 0) {
			LUA_checkStack2(lua.getState());
			lua_State* L = lua.getState();
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			Layer layer = LAYER_BACK;
			while (lua_next(L, -2)) {
				if (!lua_istable(lua.getState(), -1)) {
					Log::error(LOG_COMMON, "spritedef: expected frame table on the stack: %s", lua.getStackDump().c_str());
					lua.pop();
					continue;
				}
				// push the frame table
				const int framesOnLayer = lua_rawlen(lua.getState(), -1);
				Log::debug(LOG_COMMON, "id: %s => %i frames on layer %i", id.c_str(), framesOnLayer, layer);
				for (int i = 1; i <= framesOnLayer; ++i) {
					const std::string& texture = lua.getTableString(i);
					Log::debug(LOG_COMMON, "id: %s => texture %s on layer %i", id.c_str(), texture.c_str(), layer);
					const SpriteDefFrame frame(texture, 0, true);
					if (layer < MAX_LAYERS)
						def->textures[layer].push_back(frame);
				}
				lua.pop();
				++layer;
			}
			// pop the frame table
			lua.pop();
		}

		// pop the frames table
		if (layers != -1)
			lua.pop();

		// push the polygons table
		const int polygons = lua.getTable("polygons");
		Log::debug(LOG_COMMON, "id: %s => %i polygons", id.c_str(), polygons);
		if (polygons > 0) {
			LUA_checkStack2(lua.getState());
			lua_State* L = lua.getState();
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				if (!lua_istable(L, -1)) {
					Log::error(LOG_COMMON, "spritedef: expected polygon table on the stack: %s", lua.getStackDump().c_str());
					lua.pop();
					continue;
				}
				// push the polygon table
				const int vertices = lua_rawlen(L, -1) - 1;
				Log::debug(LOG_COMMON, "id: %s => %i vertices", id.c_str(), vertices);
				const std::string& userData = lua.getTableString(1);
				SpritePolygon p(userData);
				Log::debug(LOG_COMMON, "id: %s => %s userdata", id.c_str(), userData.c_str());
				for (int i = 2; i <= vertices; i += 2) {
					const float x = lua.getTableFloat(i) / 100.0f;
					const float y = lua.getTableFloat(i + 1) / 100.0f;
					p.vertices.push_back(SpriteVertex(x, y));
				}
				// pop the polygon table
				lua.pop();
				def->polygons.push_back(p);
			}
			lua.pop();
		}

		// pop the polygons table
		if (polygons != -1)
			lua.pop();

		// push the circles table
		const int circles = lua.getTable("circles");
		Log::debug(LOG_COMMON, "id: %s => %i circles", id.c_str(), circles);
		if (circles > 0) {
			LUA_checkStack2(lua.getState());
			lua_State* L = lua.getState();
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				if (!lua_istable(L, -1)) {
					Log::error(LOG_COMMON, "spritedef: expected circle table on the stack: %s", lua.getStackDump().c_str());
					lua.pop();
					continue;
				}
				// push the circle table
				const int entries = lua_rawlen(lua.getState(), -1);
				if (entries == 4) {
					const std::string& userData = lua.getTableString(1);
					SpriteCircle p(userData);
					const float x = lua.getTableInteger(2) / 100.0f;
					const float y = lua.getTableInteger(3) / 100.0f;
					p.center = SpriteVertex(x, y);
					p.radius = lua.getTableInteger(4) / 100.0f;
					def->circles.push_back(p);
				} else {
					Log::error(LOG_COMMON, "invalid amount of entries for the circle shape");
				}
				// pop the circle table
				lua.pop();
			}
			lua.pop();
		}
		// pop the circles table
		if (circles != -1)
			lua.pop();

		for (Layer layer = LAYER_BACK; layer <= MAX_LAYERS; layer++) {
			if (layer == MAX_LAYERS || def->textures[layer].empty()) {
				int frame = 1;
				std::string spriteFrameName = def->id;
				switch (layer) {
				case LAYER_BACK:
					spriteFrameName += "-back-01";
					break;
				case LAYER_FRONT:
					spriteFrameName += "-front-01";
					break;
				case LAYER_MIDDLE:
					spriteFrameName += "-middle-01";
					break;
				case MAX_LAYERS:
					// fallback
					spriteFrameName += "-01";
					break;
				default:
					break;
				}
				const size_t length = spriteFrameName.size();
				char frameNumbers[] = { '0', '1', '\0' };
				Layer layerToUse = layer;
				if (layerToUse == MAX_LAYERS)
					layerToUse = LAYER_MIDDLE;
				for (;;) {
					if (!textureDefinition.exists(spriteFrameName))
						break;

					def->textures[layerToUse].push_back(SpriteDefFrame(spriteFrameName, 0, true));

					frame++;
					const char first = frame / 10 + '0';
					const char second = frame % 10 + '0';
					frameNumbers[0] = first;
					frameNumbers[1] = second;
					spriteFrameName.replace(length - 2, length, frameNumbers);
				}
			}
		}

		const int actives = lua.getTable("active");
		for (int i = 1; i <= actives; ++i) {
			LUA_checkStack2(lua.getState());
			const bool active = lua.getTableBool(i);
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
				const int textures = def->textures[layer].size();
				if (textures < i)
					continue;
				SpriteDef::SpriteDefFrames& frames = def->textures[layer];
				frames[i - 1].active = active;
			}
		}
		// pop the active table
		if (actives != -1)
			lua.pop();

		const int delays = lua.getTable("delays");
		for (int i = 1; i <= delays; ++i) {
			LUA_checkStack2(lua.getState());
			const int delay = lua.getTableInteger(i);
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
				const int textures = def->textures[layer].size();
				if (textures >= i)
					def->textures[layer][i - 1].delay = delay;
			}
		}
		// pop the delays table
		if (delays != -1)
			lua.pop();

		lua.pop();

		def->calcDelay();

		_spriteDefs[id] = SpriteDefPtr(def);
	}
}

SpriteDefinition::~SpriteDefinition ()
{
}

SpriteDefinition& SpriteDefinition::get ()
{
	static SpriteDefinition instance;
	return instance;
}

SpriteDefPtr SpriteDefinition::getFromEntityType (const EntityType& entityType, const Animation& animation) const
{
	const std::string baseName = getSpriteName(entityType, animation);
	const std::string name = string::replaceAll(baseName, TEXTURE_DIRECTION, TEXTURE_DIRECTION_RIGHT);
	const SpriteDefPtr& def = getSpriteDefinition(name);
	if (!def) {
		for (SpriteDefMapConstIter i = _spriteDefs.begin(); i != _spriteDefs.end(); ++i) {
			if (i->first.compare(0, name.size(), name) == 0) {
				return i->second;
			}
		}
	}
	return def;
}

SpriteDefPtr SpriteDefinition::getSpriteDefinition (const std::string& spriteName) const
{
	if (spriteName.empty())
		return SpriteDefPtr();
	SpriteDefMapConstIter i = _spriteDefs.find(spriteName);
	if (i == _spriteDefs.end()) {
		Log::error(LOG_COMMON, "could not find sprite definition for %s", spriteName.c_str());
		for (SpriteDefMapConstIter iter = _spriteDefs.begin(); iter != _spriteDefs.end(); ++iter) {
			Log::error(LOG_COMMON, " + found: %s", iter->first.c_str());
		}
		return SpriteDefPtr();
	}

	if (!i->second->redirect.empty()) {
		i = _spriteDefs.find(spriteName);
		if (i == _spriteDefs.end()) {
			Log::error(LOG_COMMON, "could not find sprite redirect definition for %s", spriteName.c_str());
			for (SpriteDefMapConstIter iter = _spriteDefs.begin(); iter != _spriteDefs.end(); ++iter) {
				Log::error(LOG_COMMON, " + found: %s", iter->first.c_str());
			}
			return SpriteDefPtr();
		}
	}
	return i->second;
}

std::string SpriteDefinition::getSpriteName (const EntityType& type, const Animation& animation) const
{
	if (animation.isNone())
		return type.name;

	if (animation.name.empty() || type.name.empty())
		return "";

	return type.name + "-" + animation.name;
}

int SpriteDefinition::getAnimationLengthFromDef (const SpriteDefPtr& spriteDef) const
{
	int max = 0;
	const int fps = spriteDef->fps;
	const int l = 1000 / fps;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
		const SpriteDef::SpriteDefFrames& frames = spriteDef->textures[layer];
		const int layerDuration = l * frames.size() + spriteDef->delay;
		if (max < layerDuration)
			max = layerDuration;
	}
	return max;
}

int SpriteDefinition::getAnimationLength (const EntityType& type, const Animation& animation) const
{
	const std::string spriteName = getSpriteName(type, animation);
	const SpriteDefPtr spriteDef = getSpriteDefinition(spriteName);
	if (!spriteDef)
		return -1;

	return getAnimationLengthFromDef(spriteDef);
}

bool SpriteDefinition::isFrameActive (uint32_t time, const SpriteDefPtr spriteDef) const
{
	const int animationLength = getAnimationLengthFromDef(spriteDef);
	if (animationLength <= 0)
		return false;
	const int currentFrameTime = time % animationLength;
	const int fps = spriteDef->fps;
	const int l = 1000 / fps;
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
		int frameTimeEnd = 0;
		const SpriteDef::SpriteDefFrames& frames = spriteDef->textures[layer];
		for (SpriteDef::TexturesConstIter i = frames.begin(); i != frames.end(); ++i) {
			// delay and active are set for all the layers
			frameTimeEnd += l + i->delay;
			if (currentFrameTime < frameTimeEnd) {
				return i->active;
			}
		}
	}
	return false;
}

vec2 SpriteDef::getMins () const
{
	float mins[] = { 0.0f, 0.0f };
	// create the shape from the sprite definition polygons and circles
	for (std::vector<SpriteCircle>::const_iterator i = circles.begin(); i != circles.end(); ++i) {
		// TODO: the center is not taken into account here
		mins[0] = mins[1] = i->radius / -2.0f;
	}
	for (std::vector<SpritePolygon>::const_iterator i = polygons.begin(); i != polygons.end(); ++i) {
		const SpritePolygon& polygon = *i;
		const SpritePolygon::SpriteVector& polyVerts = polygon.vertices;
		for (SpritePolygon::SpriteVector::const_iterator j = polyVerts.begin(); j != polyVerts.end(); ++j) {
			const SpriteVertex &v = *j;
			for (int n = 0; n < 2; ++n) {
				const float val = n == 0 ? v.x : v.y;
				if (val < mins[n])
					mins[n] = val;
			}
		}
	}
	return vec2(mins[0] + 0.5f, mins[1] + 0.5f);
}

vec2 SpriteDef::getMaxs () const
{
	float maxs[] = { 0.0f, 0.0f };
	// create the shape from the sprite definition polygons and circles
	for (std::vector<SpriteCircle>::const_iterator i = circles.begin(); i != circles.end(); ++i) {
		// TODO: the center is not taken into account here
		maxs[0] = maxs[1] = i->radius / 2.0f;
	}
	for (std::vector<SpritePolygon>::const_iterator i = polygons.begin(); i != polygons.end(); ++i) {
		const SpritePolygon& polygon = *i;
		const SpritePolygon::SpriteVector& polyVerts = polygon.vertices;
		for (SpritePolygon::SpriteVector::const_iterator j = polyVerts.begin(); j != polyVerts.end(); ++j) {
			const SpriteVertex &v = *j;
			for (int n = 0; n < 2; ++n) {
				const float val = n == 0 ? v.x : v.y;
				if (val > maxs[n])
					maxs[n] = val;
			}
		}
	}

	return vec2(maxs[0] + 0.5f, maxs[1] + 0.5f);
}

vec2 SpriteDef::calculateSizeFromShapeData () const
{
	if (_shapeSizeCalculated)
		return _cachedShapeSize;

	if (!hasShape()) {
		_cachedShapeSize = vec2_zero;
		_shapeSizeCalculated = true;
		return _cachedShapeSize;
	}

	const vec2 mins = getMins();
	const vec2 maxs = getMaxs();
	_cachedShapeSize = vec2(maxs.x - mins.x, maxs.y - mins.y);
	_shapeSizeCalculated = true;
	return _cachedShapeSize;
}
