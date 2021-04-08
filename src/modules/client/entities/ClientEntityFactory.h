#pragma once

#include "common/EntityType.h"
#include "common/Animation.h"
#include "common/EntityAlignment.h"
#include "common/SpriteDefinition.h"
#include "common/IFactoryRegistry.h"
#include <memory>
#include "sound/Sound.h"
#include "common/Singleton.h"
#include "common/Log.h"
#include "common/LUALibrary.h"
#include "common/ExecutionTime.h"
#include <string>

class ClientEntity;
typedef ClientEntity* ClientEntityPtr;

typedef std::map<const Animation*, std::string> SoundMapping;
typedef SoundMapping::const_iterator SoundMappingConstIter;
typedef std::map<const EntityType*, SoundMapping> SoundMappingCache;
typedef SoundMappingCache::const_iterator SoundMappingCacheConstIter;

class ClientEntityFactoryContext {
public:
	ClientEntityFactoryContext (const EntityType& _type, uint16_t _id, const std::string& _sprite,
			const Animation& _animation, float _x,
			float _y, float _width, float _height, EntityAngle _angle, const SoundMapping& _soundMapping, EntityAlignment _align) :
			soundMapping(_soundMapping), type(_type), id(_id), sprite(_sprite), animation(_animation), x(_x), y(_y), width(_width), height(
					_height), angle(_angle), align(_align)
	{
	}

	const SoundMapping& soundMapping;
	const EntityType& type;
	uint16_t id;
	const std::string& sprite;
	const Animation& animation;
	float x;
	float y;
	float width;
	float height;
	EntityAngle angle;
	EntityAlignment align;
};

class IClientEntityFactory: public IFactory<ClientEntity, ClientEntityFactoryContext> {
public:
	virtual ~IClientEntityFactory ()
	{
	}
	virtual ClientEntityPtr create (const ClientEntityFactoryContext *ctx) const = 0;
};

class ClientEntityRegistry: public IFactoryRegistry<const EntityType*, ClientEntity, ClientEntityFactoryContext> {
private:
	SoundMappingCache _soundMappingCache;
	friend class SoundMapper;

	void loadLUA () {
		ExecutionTime cache("Initialize entity sounds");
		LUA lua;
		if (!lua.load("entitysounds.lua")) {
			Log::error(LOG_CLIENT, "could not load entitysounds.lua script");
			return;
		}

		for (EntityType::TypeMapConstIter eIter = EntityType::begin(); eIter != EntityType::end(); ++eIter) {
			std::string typeName = eIter->second->name;
			typeName = string::replaceAll(typeName, "-", "");
			for (Animation::TypeMapConstIter aIter = Animation::begin(); aIter != Animation::end(); ++aIter) {
				const std::string animationName = string::replaceAll(aIter->second->name, "-", "");
				const std::string id = typeName + "." + animationName;
				const std::string sound = lua.getString(id, "");
				if (sound.empty())
					continue;
				_soundMappingCache[eIter->second][aIter->second] = sound;
			}
		}
	}
public:
	ClientEntityRegistry () {}

	void initSounds() {
		loadLUA();
		for (SoundMappingCacheConstIter i = _soundMappingCache.begin(); i != _soundMappingCache.end(); ++i) {
			ExecutionTime cache("Entity sound cache: " + i->first->name + " (" + string::toString(i->second.size()) + ")");
			for (SoundMappingConstIter innerI = i->second.begin(); innerI != i->second.end(); ++innerI) {
				SoundControl.cache(innerI->second);
			}
		}
	}
	static ClientEntityPtr get (const EntityType& type, uint16_t id, const std::string& sprite = "",
			const Animation& animation = Animation::NONE, float x = 0.0f, float y = 0.0f, float sizeX = 0.0f,
			float sizeY = 0.0f, EntityAngle angle = 0, EntityAlignment align = ENTITY_ALIGN_LOWER_LEFT)	{
		ClientEntityRegistry& reg = Singleton<ClientEntityRegistry>::getInstance();
		const ClientEntityFactoryContext ctx(type, id, sprite, animation, x, y, sizeX, sizeY, angle, reg._soundMappingCache[&type], align);
		return reg.create(&type, &ctx);
	}
};
