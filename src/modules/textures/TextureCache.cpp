#include "TextureCache.h"
#include "common/Log.h"
#include "common/ExecutionTime.h"
#include "common/TextureDefinition.h"
#include "service/ServiceProvider.h"

TextureCache::TextureCache () :
		_frontend(nullptr)
{
}

TextureCache::~TextureCache ()
{
}

void TextureCache::create (const std::string& textureName, const std::string& id, const TextureDefinitionCoords& texcoords,
		const TextureDefinitionTrim& trim, bool mirror)
{
	if (_textureDefs.find(id) != _textureDefs.end()) {
		Log::error(LOG_TEXTURES, "texture def with same name found: %s", id.c_str());
		return;
	}

	TexturePtr base(load(textureName));
	Texture *t = new Texture(*base.get());
	const int x = round(texcoords.x0 * t->getFullWidth());
	const int y = round(texcoords.y0 * t->getFullHeight());
	const int w = round(texcoords.x1 * t->getFullWidth());
	const int h = round(texcoords.y1 * t->getFullHeight());
	t->setRect(x, y, w, h);
	t->setMirror(mirror);
	t->setTrim(trim);
	TexturePtr p(t);
	_textureDefs[id] = p;
}

TexturePtr TextureCache::load (const std::string& textureName)
{
	if (textureName.empty())
		return TexturePtr();
	const size_t length = strlen(TEXTURE_SPECIAL_MARKER);
	if (textureName.compare(0, length, TEXTURE_SPECIAL_MARKER) == 0) {
		TextureMap::iterator iter = _textures.find(textureName);
		if (iter != _textures.end()) {
			_textures.erase(iter);
		}
	} else {
		TextureDefinitionMap::iterator i = _textureDefs.find(textureName);
		if (i != _textureDefs.end()) {
			return i->second;
		}

		TextureMap::iterator iter = _textures.find(textureName);
		if (iter != _textures.end()) {
			return iter->second;
		}
	}

	TexturePtr p(new Texture(textureName, _frontend));
	_textures[textureName] = p;
	return p;
}

void TextureCache::dump () const
{
	Log::info(LOG_TEXTURES, "textures: %i", (int)(_textures.size() + _textureDefs.size()));
	for (TextureMap::const_iterator i = _textures.begin(); i != _textures.end(); ++i) {
		Log::info(LOG_TEXTURES, " - %s: (%i:%i)", i->first.c_str(), i->second->getWidth(), i->second->getHeight());
	}
	for (TextureDefinitionMap::const_iterator i = _textureDefs.begin(); i != _textureDefs.end(); ++i) {
		Log::info(LOG_TEXTURES, " - %s: (%i:%i)", i->first.c_str(), i->second->getWidth(), i->second->getHeight());
	}
}

void TextureCache::init (IFrontend *frontend, TextureDefinition& textureDefinition)
{
	const TextureDefinition::TextureDefMap& map = textureDefinition.getMap();
	ExecutionTime e("Texture loading");
	Log::info(LOG_TEXTURES, "load %i textures", (int)map.size());

	_frontend = frontend;

	for (TextureDefinition::TextureDefMapConstIter i = map.begin(); i != map.end(); ++i) {
		const TextureDef& t = i->second;
		create(t.textureName, t.id, t.texcoords, t.trim, t.mirror);
	}
}

void TextureCache::shutdown ()
{
	Log::info(LOG_TEXTURES, "shutting down the texturecache with %i textures", (int)_textures.size());
	for (TextureMap::iterator i = _textures.begin(); i != _textures.end(); ++i) {
		i->second->deleteTexture();
	}
	_textures.clear();
	_textureDefs.clear();
}
