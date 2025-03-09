// This tool reads a TexturePacker .tps file and creates a texture atlas with the specified scale and extension.

#include <cstdio>
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

struct Image {
	std::string name;
	unsigned char *data;
	int width, height, channels;

	~Image() {
		if (data) {
			stbi_image_free(data);
		}
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

static double readDouble(const pugi::xml_node &parent, const std::string &name, double defaultVal = 0.0f) {
	const std::string xpath = "./key[text()='" + name + "']/following-sibling::double";
	pugi::xpath_node_set node = parent.select_nodes(xpath.c_str());
	if (!node.empty()) {
		double val = node.first().node().text().as_double();
		printf("%s: %f\n", name.c_str(), val);
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
		printf("%s: %s\n", name.c_str(), val.c_str());
		return val;
	}
	return defaultVal;
}

static bool handleVariant(const Variants &v, const std::vector<Image> &images) {
	std::vector<stbrp_rect> rects;
	rects.resize(images.size());
	std::vector<Image> scaledImages;
	scaledImages.resize(images.size());

	// Scale images with stb_image_resize2
	for (size_t i = 0; i < images.size(); i++) {
		const Image &image = images[i];
		const int newWidth = (int)(image.width * v.scale);
		const int newHeight = (int)(image.height * v.scale);
		scaledImages[i].data = (unsigned char *)calloc(newWidth * newHeight * 4, 1);
		if (!scaledImages[i].data) {
			printf("Failed to allocate memory for scaled image\n");
			return false;
		}
		scaledImages[i].width = newWidth;
		scaledImages[i].height = newHeight;
		scaledImages[i].channels = 4;
		const int inputImageStride = image.width * 4;
		const int outputImageStride = newWidth * 4;
		stbir_resize_uint8_linear(image.data, image.width, image.height, inputImageStride, scaledImages[i].data,
								  newWidth, newHeight, outputImageStride, STBIR_RGBA);
	}

	for (size_t i = 0; i < scaledImages.size(); i++) {
		rects[i].id = (int)i;
		rects[i].w = scaledImages[i].width;
		rects[i].h = scaledImages[i].height;
	}

	printf("use %s to save the texture atlas with a scale of %f\n", v.textureFilename.c_str(), v.scale);

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
			printf("Failed to pack rectangles with %i:%i\n", currentAtlasWidth, currentAtlasHeight);
			if (++attempt % 2 == 0) {
				currentAtlasWidth += ATLAS_INCREASE;
			} else {
				currentAtlasHeight += ATLAS_INCREASE;
			}
			continue;
		}

		// Create atlas
		const size_t atlasMemSize = ((size_t)currentAtlasWidth * (size_t)currentAtlasHeight * 4);
		uint8_t *atlas = (uint8_t *)calloc(atlasMemSize, 1);
		if (!atlas) {
			printf("Failed to allocate memory for atlas\n");
			return false;
		}

		FILE *luaFile = fopen(v.luaFilename.c_str(), "w");
		if (!luaFile) {
			printf("Failed to open %s\n", v.luaFilename.c_str());
			return false;
		}

		// Write lua file
		fprintf(luaFile, "textures = {\n");

		// Copy images into atlas
		for (size_t j = 0; j < rects.size(); j++) {
			if (!rects[j].was_packed) {
				continue;
			}
			const int x = rects[j].x, y = rects[j].y;
			const int n = scaledImages[j].width * 4;
			for (int row = 0; row < scaledImages[j].height; row++) {
				uint8_t *dest = atlas + (intptr_t)((y + row) * currentAtlasWidth + x) * 4;
				const uint8_t *src = scaledImages[j].data + (intptr_t)(row * n);
				memcpy(dest, src, n);
			}

			fprintf(luaFile, "[");
			std::string spriteId = images[j].name;
			size_t pos = spriteId.find_last_of('.');
			// remove extension
			if (pos != std::string::npos) {
				spriteId = spriteId.substr(0, pos);
			}
			// remove path
			pos = spriteId.find_last_of("/\\");
			if (pos != std::string::npos) {
				spriteId = spriteId.substr(pos + 1);
			}

			fprintf(luaFile, "\"%s\"", spriteId.c_str());
			fprintf(luaFile, "] = {\n");
			fprintf(luaFile, "\timage = \"%s\",\n", v.textureFilename.c_str());
			fprintf(luaFile, "\tx0 = %i,\n", x);
			fprintf(luaFile, "\ty0 = %i,\n", y);
			fprintf(luaFile, "\tx1 = %i,\n", x + scaledImages[j].width);
			fprintf(luaFile, "\ty1 = %i,\n", y + scaledImages[j].height);
			fprintf(luaFile, "\ttrimmedoffsetx = 0,\n"); // TODO
			fprintf(luaFile, "\ttrimmedoffsety = 0,\n"); // TODO
			fprintf(luaFile, "\ttrimmedwidth = %i,\n", scaledImages[j].width); // TODO
			fprintf(luaFile, "\ttrimmedheight = %i,\n", scaledImages[j].height); // TODO
			fprintf(luaFile, "\tuntrimmedwidth = %i,\n", images[j].width);
			fprintf(luaFile, "\tuntrimmedheight = %i,\n", images[j].height);
			fprintf(luaFile, "},\n");
		}
		fprintf(luaFile, "}\n");

		fclose(luaFile);

		// Save the atlas
		stbi_write_png(v.textureFilename.c_str(), currentAtlasWidth, currentAtlasHeight, 4, atlas,
					   currentAtlasWidth * 4);
		printf("Texture atlas saved as %s\n", v.textureFilename.c_str());

		free(atlas);
		return true;
	}
	return false;
}

static bool loadTps(const std::string &tpsFile) {
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(tpsFile.c_str());
	if (!result) {
		printf("Failed to load %s\n", tpsFile.c_str());
		return false;
	}

	const pugi::xml_node &dataStructNode = doc.child("data").child("struct");
	const std::string &textureFilename = readString(dataStructNode, "textureFileName", "", "filename");
	if (textureFilename.empty()) {
		printf("No textureFileName found\n");
		return false;
	}

	const pugi::xpath_node &luaNode = doc.select_node("//key[text()='lua']/following-sibling::struct/filename");
	const std::string &luaFilename = luaNode.node().text().as_string();
	printf("Lua file: %s\n", luaFilename.c_str());

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
	}
	if (variants.empty()) {
		printf("No variants found\n");
		return false;
	}

	const pugi::xpath_node_set &tpsFiles =
		doc.select_nodes("//key[text()='fileList']/following-sibling::array/filename");
	std::vector<std::string> input_files;
	for (const pugi::xpath_node &file : tpsFiles) {
		const std::string &imageName = file.node().text().as_string();
		printf("Image file: %s\n", imageName.c_str());
		input_files.push_back(imageName);
	}

	std::vector<Image> images;
	images.resize(input_files.size());

	for (size_t i = 0; i < input_files.size(); i++) {
		images[i].name = input_files[i];
		images[i].data = stbi_load(input_files[i].c_str(), &images[i].width, &images[i].height, &images[i].channels, 4);
		if (!images[i].data) {
			printf("Failed to load %s\n", input_files[i].c_str());
			return false;
		}
	}

	for (const Variants &v : variants) {
		if (!handleVariant(v, images)) {
			return false;
		}
	}
	return true;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s file.tps\n", argv[0]);
		return 1;
	}
	const std::string tpsFile = argv[1];

	// extract path
	std::string path = tpsFile;
	size_t pos = path.find_last_of("/\\");
	if (pos != std::string::npos) {
		path = path.substr(0, pos);
	}

	// chdir to the path
	if (chdir(path.c_str()) != 0) {
		printf("Failed to chdir to %s\n", path.c_str());
		return 1;
	}
	// extract the filename
	std::string filename = tpsFile.substr(pos + 1);
	if (!loadTps(filename)) {
		return 1;
	}
	return 0;
}
