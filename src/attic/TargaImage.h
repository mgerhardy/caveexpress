#pragma once

#include <string>
#include <SDL.h>

#define TGA_UNMAP_COMP			10
#define TGA_CHANNELS			3

class TargaImage {
public:
	void write (const std::string& filename, const unsigned char *buffer, int width, int height)
	{
		const std::string out = filename + ".tga";
		write(SDL_RWFromFile(out.c_str(), "wb"), buffer, width, height);
	}

	void writeAlpha (const std::string& filename, const unsigned char *buffer, int width, int height)
	{
		const std::string out = filename + ".tga";
		writeAlpha(SDL_RWFromFile(out.c_str(), "wb"), buffer, width, height);
	}

	/**
	 * @brief Writes BGRA targa image files
	 */
	void writeAlpha (SDL_RWops *dst, const unsigned char *buffer, int width, int height)
	{
		unsigned char header[18];
		char footer[26];

		memset(&header, 0, sizeof(header));

		/* Fill in header */
		/* unsigned char 0: image ID field length */
		/* unsigned char 1: color map type */
		header[2] = 2; /* image type: uncompressed */
		/* unsigned char 3 - 11: palette data */
		/* image width */
		header[12] = width & 255;
		header[13] = (width >> 8) & 255;
		/* image height */
		header[14] = height & 255;
		header[15] = (height >> 8) & 255;

		/* save the alpha channel */
		header[16] = 32; /* pixel size */
		header[17] = 8; /* 8 bits of alpha */

		SDL_RWwrite(dst, &header, sizeof(header), 1);

		/* flip upside down */
		for (int y = height - 1; y >= 0; y--) {
			for (int i = 0; i < width * 4; i += 4) {
				SDL_RWwrite(dst, buffer + y * width * 4 + i + 2, 1, 1);
				SDL_RWwrite(dst, buffer + y * width * 4 + i + 1, 1, 1);
				SDL_RWwrite(dst, buffer + y * width * 4 + i + 0, 1, 1);
				SDL_RWwrite(dst, buffer + y * width * 4 + i + 3, 1, 1);
			}
		}

		/* write footer (optional, but the specification recommends it) */
		strncpy(&footer[8], "TRUEVISION-XFILE", 16);
		footer[24] = '.';
		footer[25] = 0;
		SDL_RWwrite(dst, footer, sizeof(footer), 1);
	}

	/**
	 * @brief Writes RGB data
	 */
	void write (SDL_RWops *dst, const unsigned char *buffer, int width, int height)
	{
		unsigned char pixel_data[TGA_CHANNELS];
		unsigned char block_data[TGA_CHANNELS * 128];
		unsigned char rle_packet;
		int compress = 0;
		size_t block_length = 0;
		unsigned char header[18];
		char footer[26];

		int y;
		size_t x;

		memset(&header, 0, sizeof(header));
		memset(&footer, 0, sizeof(footer));

		/* Fill in header */
		/* unsigned char 0: image ID field length */
		/* unsigned char 1: color map type */
		header[2] = TGA_UNMAP_COMP; /* image type: Truecolor RLE encoded */
		/* unsigned char 3 - 11: palette data */
		/* image width */
		header[12] = width & 255;
		header[13] = (width >> 8) & 255;
		/* image height */
		header[14] = height & 255;
		header[15] = (height >> 8) & 255;
		header[16] = 8 * TGA_CHANNELS; /* Pixel size 24 (RGB) or 32 (RGBA) */
		header[17] = 0x20; /* Origin at bottom left */

		/* write header */
		SDL_RWwrite(dst, &header, sizeof(header), 1);

		for (y = height - 1; y >= 0; y--) {
			for (x = 0; x < width; x++) {
				const size_t index = y * width * TGA_CHANNELS + x * TGA_CHANNELS;
				pixel_data[0] = buffer[index + 2];
				pixel_data[1] = buffer[index + 1];
				pixel_data[2] = buffer[index];

				if (block_length == 0) {
					memcpy(block_data, pixel_data, TGA_CHANNELS);
					block_length++;
					compress = 0;
				} else {
					if (!compress) {
						/* uncompressed block and pixel_data differs from the last pixel */
						if (memcmp(&block_data[(block_length - 1) * TGA_CHANNELS], pixel_data, TGA_CHANNELS) != 0) {
							/* append pixel */
							memcpy(&block_data[block_length * TGA_CHANNELS], pixel_data, TGA_CHANNELS);

							block_length++;
						} else {
							/* uncompressed block and pixel data is identical */
							if (block_length > 1) {
								/* write the uncompressed block */
								rle_packet = block_length - 2;
								SDL_RWwrite(dst, &rle_packet, 1, 1);
								SDL_RWwrite(dst, block_data, (block_length - 1) * TGA_CHANNELS, 1);
								block_length = 1;
							}
							memcpy(block_data, pixel_data, TGA_CHANNELS);
							block_length++;
							compress = 1;
						}
					} else {
						/* compressed block and pixel data is identical */
						if (memcmp(block_data, pixel_data, TGA_CHANNELS) == 0) {
							block_length++;
						} else {
							/* compressed block and pixel data differs */
							if (block_length > 1) {
								/* write the compressed block */
								rle_packet = block_length + 127;
								SDL_RWwrite(dst, &rle_packet, 1, 1);
								SDL_RWwrite(dst, block_data, TGA_CHANNELS, 1);
								block_length = 0;
							}
							memcpy(&block_data[block_length * TGA_CHANNELS], pixel_data, TGA_CHANNELS);
							block_length++;
							compress = 0;
						}
					}
				}

				if (block_length == 128) {
					rle_packet = block_length - 1;
					if (!compress) {
						SDL_RWwrite(dst, &rle_packet, 1, 1);
						SDL_RWwrite(dst, block_data, 128 * TGA_CHANNELS, 1);
					} else {
						rle_packet += 128;
						SDL_RWwrite(dst, &rle_packet, 1, 1);
						SDL_RWwrite(dst, block_data, TGA_CHANNELS, 1);
					}

					block_length = 0;
					compress = 0;
				}
			}
		}

		/* write remaining bytes */
		if (block_length) {
			rle_packet = block_length - 1;
			if (!compress) {
				SDL_RWwrite(dst, &rle_packet, 1, 1);
				SDL_RWwrite(dst, block_data, block_length * TGA_CHANNELS, 1);
			} else {
				rle_packet += 128;
				SDL_RWwrite(dst, &rle_packet, 1, 1);
				SDL_RWwrite(dst, block_data, TGA_CHANNELS, 1);
			}
		}

		/* write footer (optional, but the specification recommends it) */
		strncpy(&footer[8], "TRUEVISION-XFILE", 16);
		footer[24] = '.';
		footer[25] = 0;
		SDL_RWwrite(dst, footer, sizeof(footer), 1);
	}
};
