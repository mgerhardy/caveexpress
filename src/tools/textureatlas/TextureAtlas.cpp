// This tool reads a TexturePacker .tps file and creates a texture atlas with the specified scale and extension.

#include <algorithm>
#include <cstdio>
#include <limits>
#include <string>
#include <unistd.h>
#include <vector>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "pugixml.hpp"

#include <stdio.h>
#include <stdlib.h>

static const int ATLAS_INCREASE = 32;
static bool debug = false;

struct Image {
	std::string name;
	uint8_t *data;
	int width, height, channels;

	Image() : data(nullptr), width(0), height(0), channels(0) {
	}

	~Image() {
		if (data) {
			stbi_image_free(data);
		}
	}

	Image(Image &&other) {
		name = std::move(other.name);
		data = other.data;
		width = other.width;
		height = other.height;
		channels = other.channels;
		other.data = nullptr;
	}

	Image &operator=(Image &&other) {
		if (this != &other) {
			name = std::move(other.name);
			data = other.data;
			width = other.width;
			height = other.height;
			channels = other.channels;
			other.data = nullptr;
		}
		return *this;
	}
};

struct Variants {
	Variants(double scaleVal, const std::string &extensionVal, const std::string &textureFilenameVal,
			 const std::string &luaFilenameVal)
		: scale(scaleVal), extension(extensionVal), textureFilename(textureFilenameVal), luaFilename(luaFilenameVal) {
	}
	double scale = 1.0f;
	std::string extension;
	std::string textureFilename;
	std::string luaFilename;
};

static std::string extractBasenameNoExtension(const std::string &in) {
	std::string str = in;
	size_t pos = str.find_last_of('.');
	// remove extension
	if (pos != std::string::npos) {
		str = str.substr(0, pos);
	}
	// remove path
	pos = str.find_last_of("/\\");
	if (pos != std::string::npos) {
		str = str.substr(pos + 1);
	}
	return str;
}

// TODO: add gcc printf format checks
static void debug_printf(const char *format, ...) {
	if (debug) {
		va_list args;
		va_start(args, format);
		printf("DEBUG: ");
		vprintf(format, args);
		va_end(args);
	}
}

static void error_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, format, args);
	va_end(args);
}

static double readDouble(const pugi::xml_node &parent, const std::string &name, double defaultVal = 0.0f) {
	const std::string xpath = "./key[text()='" + name + "']/following-sibling::double";
	pugi::xpath_node_set node = parent.select_nodes(xpath.c_str());
	if (!node.empty()) {
		double val = node.first().node().text().as_double();
		debug_printf("%s: %f\n", name.c_str(), val);
		return val;
	}
	return defaultVal;
}

static std::string readString(const pugi::xml_node &node, const std::string &name, const std::string &defaultVal = "",
							  const std::string &type = "string") {
	const std::string xpath = "./key[text()='" + name + "']/following-sibling::" + type;
	pugi::xpath_node_set nodeSet = node.select_nodes(xpath.c_str());
	if (!nodeSet.empty()) {
		std::string val = nodeSet.first().node().text().as_string();
		debug_printf("%s: %s\n", name.c_str(), val.c_str());
		return val;
	}
	return defaultVal;
}

struct Rect {
	int x, y, width, height;
};

static Rect findOpaqueRect(const uint8_t *data, int width, int height, int channels, int alphaThreshold = 0) {
	if (channels < 4) {
		return {0, 0, width, height};
	}

	int minX = std::numeric_limits<int>::max();
	int minY = std::numeric_limits<int>::max();
	int maxX = std::numeric_limits<int>::min();
	int maxY = std::numeric_limits<int>::min();

	for (int y = 0; y < height; ++y) {
		const int stride = y * width;
		for (int x = 0; x < width; ++x) {
			const int index = (stride + x) * channels;
			const uint8_t alpha = data[index + 3]; // Alpha channel
			if (alpha <= alphaThreshold) {
				continue;
			}
			if (x < minX) {
				minX = x;
			}
			if (y < minY) {
				minY = y;
			}
			if (x > maxX) {
				maxX = x;
			}
			if (y > maxY) {
				maxY = y;
			}
		}
	}

	if (minX == std::numeric_limits<int>::max()) {
		// No opaque pixels found
		return {0, 0, 0, 0};
	}

	return {minX, minY, maxX - minX + 1, maxY - minY + 1};
}

struct Config {
	int alphaThreshold = 0;
};

static bool handleVariant(const Variants &v, const std::vector<Image> &images, const Config &cfg) {
	std::vector<stbrp_rect> rects;
	rects.resize(images.size());
	std::vector<Image> scaledImages;
	scaledImages.resize(images.size());

	printf("* Generate texture atlas for variant: %s (scale: %f)\n", v.extension.c_str(), v.scale);
	printf("  * Save texture atlas at %s\n", v.textureFilename.c_str());
	printf("  * Save lua file: %s\n", v.luaFilename.c_str());

	for (size_t i = 0; i < images.size(); i++) {
		const Image &image = images[i];
		const int newWidth = (int)(image.width * v.scale);
		const int newHeight = (int)(image.height * v.scale);
		scaledImages[i].data = (uint8_t *)calloc(newWidth * newHeight * 4, 1);
		if (!scaledImages[i].data) {
			error_printf("Failed to allocate memory for scaled image\n");
			return false;
		}
		scaledImages[i].name = image.name;
		scaledImages[i].width = newWidth;
		scaledImages[i].height = newHeight;
		scaledImages[i].channels = 4;
		const int inputImageStride = image.width * 4;
		const int outputImageStride = newWidth * 4;
		if (fabs(v.scale - 1.0) < 0.0001) {
			memcpy(scaledImages[i].data, image.data, newWidth * newHeight * 4);
		} else {
			stbir_resize_uint8_linear(image.data, image.width, image.height, inputImageStride, scaledImages[i].data,
									  newWidth, newHeight, outputImageStride, STBIR_RGBA);
			printf("  * Scaled image: %s, %i:%i -> %i:%i\n", image.name.c_str(), image.width, image.height, newWidth,
				   newHeight);
		}
		rects[i].id = (int)i;
		rects[i].w = scaledImages[i].width;
		rects[i].h = scaledImages[i].height;
	}

	int currentAtlasWidth = 256;
	int currentAtlasHeight = 256;
	int attempt = 0;
	for (int i = 0; i < 100; ++i) {
		// Initialize packing context
		stbrp_context ctx;
		std::vector<stbrp_node> nodes;
		nodes.resize(currentAtlasWidth);
		stbrp_init_target(&ctx, currentAtlasWidth, currentAtlasHeight, nodes.data(), currentAtlasWidth);

		// Pack rectangles
		if (!stbrp_pack_rects(&ctx, rects.data(), (int)rects.size())) {
			if (++attempt % 2 == 0) {
				currentAtlasWidth += ATLAS_INCREASE;
			} else {
				currentAtlasHeight += ATLAS_INCREASE;
			}
			continue;
		}
		printf("  * Texture atlas resolution %i:%i\n", currentAtlasWidth, currentAtlasHeight);

		// Create atlas
		const size_t atlasMemSize = ((size_t)currentAtlasWidth * (size_t)currentAtlasHeight * 4);
		uint8_t *atlas = (uint8_t *)calloc(atlasMemSize, 1);
		if (!atlas) {
			error_printf("Failed to allocate memory for atlas\n");
			return false;
		}

		FILE *luaFile = fopen(v.luaFilename.c_str(), "w");
		if (!luaFile) {
			error_printf("Failed to open lua output file: %s\n", v.luaFilename.c_str());
			return false;
		}

		const std::string &baseTextureName = extractBasenameNoExtension(v.textureFilename);

		// Write lua file
		fprintf(luaFile, "textures = {\n");

		// Copy images into atlas
		for (size_t j = 0; j < rects.size(); j++) {
			if (!rects[j].was_packed) {
				continue;
			}
			const Rect opaqueRect = findOpaqueRect(scaledImages[j].data, scaledImages[j].width, scaledImages[j].height,
												   4, cfg.alphaThreshold);

			const int x = rects[j].x, y = rects[j].y;
			const int n = scaledImages[j].width * 4;

			// TODO: only copy the opaque part of the image?
			for (int row = 0; row < scaledImages[j].height; row++) {
				uint8_t *dest = atlas + (intptr_t)((y + row) * currentAtlasWidth + x) * 4;
				const uint8_t *src = scaledImages[j].data + (intptr_t)(row * n);
				memcpy(dest, src, n);
			}

			const std::string &spriteId = extractBasenameNoExtension(scaledImages[j].name);

			fprintf(luaFile, "\t[");
			fprintf(luaFile, "\"%s\"", spriteId.c_str());
			fprintf(luaFile, "] = {\n");
			fprintf(luaFile, "\t\timage = \"%s\",\n", baseTextureName.c_str());
			fprintf(luaFile, "\t\tx0 = %f,\n", (float)x / (float)currentAtlasWidth);
			fprintf(luaFile, "\t\ty0 = %f,\n", (float)y / (float)currentAtlasHeight);
			fprintf(luaFile, "\t\tx1 = %f,\n", (float)scaledImages[j].width / (float)currentAtlasWidth);
			fprintf(luaFile, "\t\ty1 = %f,\n", (float)scaledImages[j].height / (float)currentAtlasHeight);
			fprintf(luaFile, "\t\ttrimmedoffsetx = %i,\n", opaqueRect.x);
			fprintf(luaFile, "\t\ttrimmedoffsety = %i,\n", opaqueRect.y);
			fprintf(luaFile, "\t\ttrimmedwidth = %i,\n", opaqueRect.width);
			fprintf(luaFile, "\t\ttrimmedheight = %i,\n", opaqueRect.height);
			fprintf(luaFile, "\t\tuntrimmedwidth = %i,\n", scaledImages[j].width);
			fprintf(luaFile, "\t\tuntrimmedheight = %i,\n", scaledImages[j].height);
			fprintf(luaFile, "\t},\n");
		}
		fprintf(luaFile, "}\n");
		fclose(luaFile);

		// Save the atlas
		stbi_write_png(v.textureFilename.c_str(), currentAtlasWidth, currentAtlasHeight, 4, atlas,
					   currentAtlasWidth * 4);
		free(atlas);
		return true;
	}
	error_printf("Failed to pack texture atlas\n");
	return false;
}

static bool loadTps(const std::string &tpsFile) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(tpsFile.c_str());
	if (!result) {
		error_printf("Failed to load %s\n", tpsFile.c_str());
		return false;
	}

	const pugi::xml_node &dataStructNode = doc.child("data").child("struct");
	const std::string &textureFilename = readString(dataStructNode, "textureFileName", "", "filename");
	if (textureFilename.empty()) {
		error_printf("No textureFileName found\n");
		return false;
	}

	Config cfg;
	cfg.alphaThreshold = 1; // TODO: read cleanTransparentPixels (The rgb values of transparent pixels are set to 0)
							// TODO: read reduceBorderArtifacts (Alpha bleeding)

	const pugi::xpath_node &luaNode = doc.select_node("//key[text()='lua']/following-sibling::struct/filename");
	const std::string &luaFilename = luaNode.node().text().as_string();
	printf("* Lua file template: %s\n", luaFilename.c_str());
	printf("* Texture file template: %s\n", textureFilename.c_str());

	pugi::xpath_node_set autoSDSettingsStructs =
		doc.select_nodes("//key[text()='autoSDSettings']/following-sibling::array/struct");

	std::vector<Variants> variants;
	variants.reserve(autoSDSettingsStructs.size());
	for (const pugi::xpath_node &node : autoSDSettingsStructs) {
		const pugi::xml_node &structNode = node.node();
		const double scale = readDouble(structNode, "scale", 1.0f);
		const std::string &extension = readString(structNode, "extension");
		// replace {v} in textureFilename and store it in textureFilenameVariant
		std::string textureFilenameVariant = textureFilename;
		size_t pos = textureFilenameVariant.find("{v}");
		if (pos != std::string::npos) {
			textureFilenameVariant.replace(pos, 3, extension);
		}
		// replace {v} in luaFilename and store it in luaFilenameVariant
		std::string luaFilenameVariant = luaFilename;
		pos = luaFilenameVariant.find("{v}");
		if (pos != std::string::npos) {
			luaFilenameVariant.replace(pos, 3, extension);
		}
		variants.emplace_back(scale, extension, textureFilenameVariant, luaFilenameVariant);
		printf("* Found variant: scale: %f, extension: %s\n", scale, extension.c_str());
	}
	if (variants.empty()) {
		error_printf("No variants found\n");
		return false;
	}

	const pugi::xpath_node_set &tpsFiles =
		doc.select_nodes("//key[text()='fileList']/following-sibling::array/filename");
	std::vector<std::string> input_files;
	printf("* Images:\n");
	for (const pugi::xpath_node &file : tpsFiles) {
		const std::string &imageName = file.node().text().as_string();
		printf("  * Image file: %s\n", imageName.c_str());
		input_files.push_back(imageName);
	}

	std::vector<Image> images;
	images.resize(input_files.size());

	for (size_t i = 0; i < input_files.size(); i++) {
		images[i].name = input_files[i];
		images[i].data = stbi_load(input_files[i].c_str(), &images[i].width, &images[i].height, &images[i].channels, 4);
		if (!images[i].data) {
			error_printf("Failed to load %s\n", input_files[i].c_str());
			return false;
		}
		// TODO: calculate hash to check for duplicates
	}

	std::sort(images.begin(), images.end(), [](const Image &a, const Image &b) {
		return extractBasenameNoExtension(a.name) < extractBasenameNoExtension(b.name);
	});

	for (const Variants &v : variants) {
		if (!handleVariant(v, images, cfg)) {
			return false;
		}
	}
	return true;
}

static void usage(const char *appname) {
	fprintf(stderr, "Usage: %s [-d] [-h] [file.tps]...\n\n", appname);
	fprintf(stderr, "This tool reads a TexturePacker .tps file and creates the texture atlas.\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	const char *appname = argv[0];
	size_t optsParsed = 1;
	for (optsParsed = 1; optsParsed < argc && argv[optsParsed][0] == '-'; optsParsed++) {
		switch (argv[optsParsed][1]) {
		case 'd':
			debug = true;
			break;
		case 'h':
		default:
			usage(appname);
		}
	}

	if (argc < 2) {
		usage(appname);
	}

	argv += optsParsed;
	argc -= (int)optsParsed;

	if (argc == 0) {
		usage(appname);
	}

	for (int i = 0; i < argc; i++) {
		const std::string tpsFile = argv[0];
		printf("Processing %s\n", tpsFile.c_str());

		// extract path
		std::string path = tpsFile;
		size_t pos = path.find_last_of("/\\");
		if (pos != std::string::npos) {
			path = path.substr(0, pos);
		}

		// chdir to the path
		if (chdir(path.c_str()) != 0) {
			error_printf("Failed to chdir to %s\n", path.c_str());
			return 1;
		}
		// extract the filename
		std::string filename = tpsFile.substr(pos + 1);
		if (!loadTps(filename)) {
			error_printf("Failed to handle %s\n", tpsFile.c_str());
			return 1;
		}
	}
	return 0;
}
