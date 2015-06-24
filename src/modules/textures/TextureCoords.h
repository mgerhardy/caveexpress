#pragma once

#include "Texture.h"

class TextureCoords {
private:
	inline void calc(const TextureRect& rect, int width, int height, bool mirror, bool flip) {
		const int x1 = rect.x;
		const int y1 = flip ? rect.y + rect.h : rect.y;
		const int x2 = rect.x + rect.w;
		const int y2 = flip ? rect.y :  rect.y + rect.h;

		texCoords[0] = x1 / (float) width;
		texCoords[1] = y1 / (float) height;

		texCoords[2] = x2 / (float) width;
		texCoords[3] = y1 / (float) height;

		texCoords[4] = x2 / (float) width;
		texCoords[5] = y2 / (float) height;

		texCoords[6] = x1 / (float) width;
		texCoords[7] = y2 / (float) height;
		if (mirror) {
			std::swap(texCoords[0], texCoords[2]);
			std::swap(texCoords[4], texCoords[6]);
		}
	}
public:
	float texCoords[8];

	TextureCoords(const Texture* texture) {
		// flip is done by ortho projection here
		calc(texture->getSourceRect(), texture->getFullWidth(), texture->getFullHeight(), texture->isMirror(), false);
	}

	TextureCoords (const float _texCoords[8]) {
		memcpy(texCoords, _texCoords, sizeof(texCoords));
	}

	TextureCoords(const TextureRect& rect, int width, int height, bool mirror = false, bool flip = false) {
		calc(rect, width, height, mirror, flip);
	}
};
