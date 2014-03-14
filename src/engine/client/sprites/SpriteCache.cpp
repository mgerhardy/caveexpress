#include "SpriteCache.h"
#include "engine/common/FileSystem.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/Logger.h"

SpriteCache::SpriteCache ()
{
}

SpriteCache::~SpriteCache ()
{
	shutdown();
}

void SpriteCache::init ()
{
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
		if (!sprite->addFrame(LAYER_DEFAULT, spriteName))
			error(LOG_CLIENT, "could not add frame '" + spriteName + "' to sprite '" + spriteName + "' (def does not exist)");
	} else {
		if (def->hasNoTextures()) {
			if (!sprite->addFrame(LAYER_DEFAULT, spriteName))
				error(LOG_CLIENT, "could not add frame '" + spriteName + "' to sprite '" + spriteName + "' (def has no textures)");
		} else {
			sprite->setFPS(def->fps);
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
				const SpriteDef::SpriteDefFrames& defFrames = def->textures[layer];
				for (SpriteDef::TexturesConstIter i = defFrames.begin(); i != defFrames.end(); ++i) {
					if (!sprite->addFrame(layer, i->name, i->delay, i->active))
						error(LOG_CLIENT, "could not add frame '" + i->name + "' to sprite '" + spriteName + "'");
				}
			}
		}
	}
	SpritePtr o = SpritePtr(sprite);
	_sprites[spriteName] = o;
	return o;
}

void SpriteCache::shutdown ()
{
	_sprites.clear();
}
