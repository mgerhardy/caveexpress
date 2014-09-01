#include "Texture.h"
#include "shared/FileSystem.h"
#include "shared/Logger.h"
#include <assert.h>
#include <SDL.h>

#define MAX_TEXTURE_SIZE 2048
#define ISPOWEROFTWO(i) ((i) > 0 && !((i) & ((i) - 1)))

Texture::Texture (const std::string &filename, IFrontend *frontend) :
		_uploadedWidth(-1), _uploadedHeight(-1), _data(nullptr), _name(filename), _copy(false), _mirror(false), _frontend(
				frontend)
{
	if (filename[0] == '*') {
		memset(&_rect, 0, sizeof(_rect));
	} else if (frontend->loadTexture(this, filename)) {
		_uploadedWidth = _rect.w;
		_uploadedHeight = _rect.h;

		debug(LOG_CLIENT, String::format("loaded texture: %s (%i:%i)", filename.c_str(), _uploadedWidth, _uploadedHeight));
	} else {
		error(LOG_CLIENT, String::format("could not load texture " + filename));
		memset(&_rect, 0, sizeof(_rect));
	}
}

Texture::Texture (const Texture& texture) :
		_uploadedWidth(texture._uploadedWidth), _uploadedHeight(texture._uploadedHeight), _data(texture._data), _name(
				texture._name), _mirror(texture._mirror), _frontend(texture._frontend), _copy(true)
{
	debug(LOG_CLIENT, String::format("copy texture %s (%i:%i)", _name.c_str(), _uploadedWidth, _uploadedHeight));
	memcpy(&_rect, &texture._rect, sizeof(_rect));
}

Texture::~Texture ()
{
	deleteTexture();
}

void Texture::deleteTexture ()
{
	if (_data && !_copy)
		_frontend->destroyTexture(_data);
	_data = nullptr;
}
