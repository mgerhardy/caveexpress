#include "SpriteCache.h"
#include "common/SpriteDefinition.h"
#include "common/Log.h"

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
			Log::error(LOG_CLIENT, "could not add frame '%s' to sprite '%s' (def does not exist)", spriteName.c_str(), spriteName.c_str());
	} else {
		if (def->hasNoTextures()) {
			if (!sprite->addFrame(LAYER_DEFAULT, spriteName))
				Log::error(LOG_CLIENT, "could not add frame '%s' to sprite '%s' (def has no textures)", spriteName.c_str(), spriteName.c_str());
		} else {
			sprite->setFPS(def->fps);
			for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; layer++) {
				const SpriteDef::SpriteDefFrames& defFrames = def->textures[layer];
				for (SpriteDef::TexturesConstIter i = defFrames.begin(); i != defFrames.end(); ++i) {
					if (!sprite->addFrame(layer, i->name, i->delay, i->active))
						Log::error(LOG_CLIENT, "could not add frame '%s'' to sprite '%s'", i->name.c_str(), spriteName.c_str());
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
