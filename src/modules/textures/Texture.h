#pragma once

#include "common/Pointers.h"
#include "common/IFrontend.h"
#include "common/TextureDefinition.h"
#include <string>
#include <stdint.h>
#include <sstream>

#define TEXTURE_SPECIAL_MARKER "***"

struct TextureRect {
	int x;
	int y;
	int w;
	int h;

	std::string toString() const {
		std::stringstream s;
		s << "TextureRect[";
		s << "x: '" << x << "', ";
		s << "y: '" << y << "', ";
		s << "w: '" << w << "', ";
		s << "h: '" << h << "'";
		s << "]";
		return s.str();
	}
};

class Texture {
private:
	int _uploadedWidth;
	int _uploadedHeight;
	const std::string _name;
	TextureRect _rect;
	bool _mirror;
	bool _copy;
	// renderer representation of the texture
	void *_data;
	IFrontend *_frontend;
	TextureDefinitionTrim _trim;

public:
	Texture (const std::string &filename, IFrontend *frontend);
	Texture (const Texture& texture);
	virtual ~Texture ();

	void setData (void *texture);
	void *getData () const;
	int getWidth () const;
	void bindTexture (int textureUnit = 0);
	int getHeight () const;
	int getFullWidth () const;
	int getFullHeight () const;

	void deleteTexture ();

	inline void setTrim (const TextureDefinitionTrim& trim)
	{
		_trim = trim;
	}

	inline const TextureDefinitionTrim& getTrim () const
	{
		return _trim;
	}

	const std::string& getName () const;

	bool isValid () const;
	void setMirror (bool mirror);
	// flip the texture horizontally
	bool isMirror () const;
	void setRect (int x, int y, int width, int height);
	const TextureRect& getSourceRect () const;
};

inline void *Texture::getData () const
{
	return _data;
}

inline int Texture::getWidth () const
{
	return _rect.w;
}

inline int Texture::getHeight () const
{
	return _rect.h;
}

inline const std::string& Texture::getName () const
{
	return _name;
}

inline void Texture::bindTexture (int textureUnit)
{
	_frontend->bindTexture(this, textureUnit);
}

inline const TextureRect& Texture::getSourceRect () const
{
	return _rect;
}

inline bool Texture::isMirror () const
{
	return _mirror;
}

inline int Texture::getFullWidth () const
{
	return _uploadedWidth;
}

inline int Texture::getFullHeight () const
{
	return _uploadedHeight;
}

inline void Texture::setData (void *texture)
{
	if (texture == nullptr && _data != nullptr && !_copy)
		_frontend->destroyTexture(_data);
	_data = texture;
}

inline void Texture::setMirror (bool mirror)
{
	_mirror = mirror;
}

inline void Texture::setRect (int x, int y, int width, int height)
{
	_rect.x = x;
	_rect.y = y;
	_rect.w = width;
	_rect.h = height;
}

inline bool Texture::isValid () const
{
	return _data != nullptr;
}

typedef SharedPtr<Texture> TexturePtr;
