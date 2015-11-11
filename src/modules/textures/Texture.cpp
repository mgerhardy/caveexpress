#include "Texture.h"
#include "common/Log.h"
#include <SDL.h>

#define MAX_TEXTURE_SIZE 2048
#define ISPOWEROFTWO(i) ((i) > 0 && !((i) & ((i) - 1)))

Texture::Texture (const std::string &filename, IFrontend *frontend) :
		_uploadedWidth(-1), _uploadedHeight(-1), _name(filename), _mirror(false), _copy(false), _data(nullptr), _frontend(
				frontend)
{
	if (filename[0] == '*') {
		memset(&_rect, 0, sizeof(_rect));
	} else if (frontend->loadTexture(this, filename)) {
		_uploadedWidth = _rect.w;
		_uploadedHeight = _rect.h;

		Log::debug(LOG_TEXTURES, "loaded texture: %s (%i:%i)", filename.c_str(), _uploadedWidth, _uploadedHeight);
	} else {
		Log::error(LOG_TEXTURES, "could not load texture %s", filename.c_str());
		memset(&_rect, 0, sizeof(_rect));
	}
}

Texture::Texture (const Texture& texture) :
		_uploadedWidth(texture._uploadedWidth), _uploadedHeight(texture._uploadedHeight), _name(
				texture._name), _mirror(texture._mirror), _copy(true), _data(texture._data), _frontend(texture._frontend)
{
	Log::trace(LOG_TEXTURES, "copy texture %s (%i:%i)", _name.c_str(), _uploadedWidth, _uploadedHeight);
	memcpy(&_rect, &texture._rect, sizeof(_rect));
}

Texture::~Texture ()
{
	deleteTexture();
}

void Texture::deleteTexture ()
{
	if (_data && !_copy) {
		Log::info(LOG_TEXTURES, "destroy texture %s", getName().c_str());
		_frontend->destroyTexture(_data);
	}
	_data = nullptr;
}
