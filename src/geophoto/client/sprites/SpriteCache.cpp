#include "SpriteCache.h"
#include "shared/FileSystem.h"
#include "shared/SpriteDefinition.h"
#include "shared/Logger.h"

SpriteCache::SpriteCache ()
{
}

SpriteCache::~SpriteCache ()
{
	_sprites.clear();
}

SpritePtr SpriteCache::load (const std::string& spriteName)
{
	SpriteMap::iterator iter = _sprites.find(spriteName);
	if (iter != _sprites.end()) {
		return iter->second;
	}

	const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(spriteName);

	Sprite *sprite = new Sprite(spriteName);
	if (!def) {
		if (!sprite->addFrame(spriteName))
			error(LOG_CLIENT, "could not add frame '" + spriteName + "' to sprite '" + spriteName + "' (def does not exist)");
	} else {
		if (def->hasNoTextures()) {
			if (!sprite->addFrame(spriteName))
				error(LOG_CLIENT, "could not add frame '" + spriteName + "' to sprite '" + spriteName + "' (def has no textures)");
		} else {
			sprite->setFPS(def->fps);
			const SpriteDef::SpriteDefFrames& defFrames = def->textures;
			for (SpriteDef::TexturesConstIter i = defFrames.begin(); i != defFrames.end(); ++i) {
				if (!sprite->addFrame(i->name, i->delay))
					error(LOG_CLIENT, "could not add frame '" + i->name + "' to sprite '" + spriteName + "'");
			}
		}
	}
	SpritePtr o = SpritePtr(sprite);
	_sprites[spriteName] = o;
	return o;
}

SpriteCache& SpriteCache::get ()
{
	static SpriteCache instance;
	return instance;
}
