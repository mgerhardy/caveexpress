#pragma once

#include "engine/client/sprites/Sprite.h"
#include "engine/common/NonCopyable.h"
#include <map>
#include <string>

class SpriteCache: public NonCopyable {
private:
	typedef std::map<std::string, SpritePtr> SpriteMap;
	typedef SpriteMap::const_iterator SpriteMapConstIter;
	SpriteMap _sprites;

public:
	SpriteCache ();
	virtual ~SpriteCache ();

	SpritePtr load (const std::string& spriteName);

	void init ();
	void shutdown ();
};
