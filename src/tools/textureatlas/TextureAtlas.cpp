// This tool reads a TexturePacker .tps file and creates a texture atlas with the specified scale and extension.
// Put into the public domain

#include "TextureAtlas.h"
#include <algorithm>
#include <cstdio>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <vector>

static int g_atlasIncrease = 32;
static bool g_debug = false;
static bool g_verbose = false;

#ifdef _WIN32
char *getcwd(char *buf, size_t size) {
	if (!buf || size == 0) {
		return nullptr;
	}

	DWORD length = GetCurrentDirectoryA(static_cast<DWORD>(size), buf);
	if (length == 0 || length >= size) {
		return nullptr; // Failed or buffer too small
	}

	return buf;
}

int chdir(const char *path) {
	return SetCurrentDirectoryA(path) ? 0 : -1;
}
#endif

struct Image {
	std::string name;
	uint8_t *data;
	uint32_t hash;
	int width, height, channels;
	int rectId;
	int duplicate;

	Image() : data(nullptr), hash(0), width(0), height(0), channels(0), rectId(-1), duplicate(-1) {
	}

	~Image() {
		if (data) {
			stbi_image_free(data);
		}
	}

	Image(Image &&other) {
		name = std::move(other.name);
		data = other.data;
		hash = other.hash;
		width = other.width;
		height = other.height;
		channels = other.channels;
		rectId = other.rectId;
		duplicate = other.duplicate;
		other.data = nullptr;
	}

	Image &operator=(Image &&other) {
		if (this != &other) {
			name = std::move(other.name);
			data = other.data;
			hash = other.hash;
			width = other.width;
			height = other.height;
			channels = other.channels;
			rectId = other.rectId;
			duplicate = other.duplicate;
			other.data = nullptr;
		}
		return *this;
	}
};

struct Variants {
	Variants(double scaleVal, const std::string &extensionVal, const std::string &textureFilenameVal,
			 const std::string &luaFilenameVal, int maxTextureWidthVal, int maxTextureHeightVal)
		: scale(scaleVal), extension(extensionVal), textureFilename(textureFilenameVal), luaFilename(luaFilenameVal),
		  maxTextureWidth(maxTextureWidthVal), maxTextureHeight(maxTextureHeightVal) {
	}
	double scale = 1.0f;
	std::string extension;
	std::string textureFilename;
	std::string luaFilename;
	int maxTextureWidth;
	int maxTextureHeight;
};

enum class TrimMode : uint8_t { None, Trim, Max };
enum class PivotPoint : uint8_t { Center, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left };

struct Config {
	int alphaThreshold = 1;
	int heuristic = STBRP_HEURISTIC_Skyline_default;
	bool pngQuant = false;
	TrimMode trimMode = TrimMode::None;
	PivotPoint pivotPoint = PivotPoint::Center;
};

struct Rect {
	int x, y, width, height;
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
	if (g_debug) {
		va_list args;
		va_start(args, format);
		printf("DEBUG: ");
		vprintf(format, args);
		va_end(args);
	}
}

static void verbose_printf(const char *format, ...) {
	if (g_verbose) {
		va_list args;
		va_start(args, format);
		printf("INFO: ");
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

static Rect findOpaqueRect(const uint8_t *data, int width, int height, int channels, const Config &cfg) {
	if (channels < 4 || cfg.trimMode == TrimMode::None) {
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
			if (alpha <= cfg.alphaThreshold) {
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

static void pngQuant(const std::string &textureFile) {
	const std::string pngQuantPath = "pngquant";
	const std::string pngQuantArgs = "--force --ext .png --speed 1 --quality 80-100 " + textureFile;
	const std::string pngQuantCmd = pngQuantPath + " " + pngQuantArgs;
	debug_printf("Run pngquant: %s\n", pngQuantCmd.c_str());
	if (system(pngQuantCmd.c_str()) != 0) {
		error_printf("Failed to run pngquant\n");
	}
}

static bool handleVariant(const Variants &v, const std::vector<Image> &images, const Config &cfg) {
	std::vector<stbrp_rect> rects;
	rects.resize(images.size());
	std::vector<Image> scaledImages;
	scaledImages.resize(images.size());

	verbose_printf("* Generate texture atlas for variant: %s (scale: %f)\n", v.extension.c_str(), v.scale);
	verbose_printf("  * Save texture atlas at %s\n", v.textureFilename.c_str());
	verbose_printf("  * Save lua file: %s\n", v.luaFilename.c_str());

	int currentRectId = 0;
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
		scaledImages[i].hash = image.hash;
		scaledImages[i].duplicate = image.duplicate;
		const int inputImageStride = image.width * 4;
		const int outputImageStride = newWidth * 4;
		if (fabs(v.scale - 1.0) < 0.0001) {
			memcpy(scaledImages[i].data, image.data, newWidth * newHeight * 4);
		} else {
			stbir_resize_uint8_linear(image.data, image.width, image.height, inputImageStride, scaledImages[i].data,
									  newWidth, newHeight, outputImageStride, STBIR_RGBA);
			verbose_printf("  * Scaled image: %s, %i:%i -> %i:%i\n", image.name.c_str(), image.width, image.height,
						   newWidth, newHeight);
		}
		if (image.duplicate < 0) {
			scaledImages[i].rectId = currentRectId;
			rects[currentRectId].id = (int)i;
			rects[currentRectId].w = scaledImages[i].width;
			rects[currentRectId].h = scaledImages[i].height;
			++currentRectId;
		} else {
			scaledImages[i].rectId = scaledImages[image.duplicate].rectId;
			assert(scaledImages[i].rectId >= 0);
		}
	}

	int currentAtlasWidth = 256;
	int currentAtlasHeight = 256;
	int attempt = 0;
	for (;;) {
		if (currentAtlasHeight > v.maxTextureHeight || currentAtlasWidth > v.maxTextureWidth) {
			error_printf("Texture atlas size exceeds maximum texture size\n");
			return false;
		}
		// Initialize packing context
		stbrp_context ctx;
		std::vector<stbrp_node> nodes;
		nodes.resize(currentAtlasWidth);
		stbrp_init_target(&ctx, currentAtlasWidth, currentAtlasHeight, nodes.data(), currentAtlasWidth);
		stbrp_setup_heuristic(&ctx, cfg.heuristic);
		// Pack rectangles
		if (!stbrp_pack_rects(&ctx, rects.data(), (int)rects.size())) {
			const int oldAtlasWidth = currentAtlasWidth;
			const int oldAtlasHeight = currentAtlasHeight;
			if (++attempt % 2 == 0 && currentAtlasWidth < v.maxTextureWidth) {
				currentAtlasWidth = (currentAtlasWidth + g_atlasIncrease) % (v.maxTextureWidth + 1);
			} else if (currentAtlasHeight < v.maxTextureHeight) {
				currentAtlasHeight = (currentAtlasHeight + g_atlasIncrease) % (v.maxTextureHeight + 1);
			}
			debug_printf("Failed to pack texture atlas, trying again with %i:%i\n", currentAtlasWidth,
						 currentAtlasHeight);
			if (currentAtlasHeight == 0 || currentAtlasWidth == 0 ||
				(currentAtlasWidth <= oldAtlasWidth && currentAtlasHeight <= oldAtlasHeight)) {
				error_printf("Failed to pack texture atlas - no more space left\n");
				return false;
			}
			continue;
		}
		verbose_printf("  * Texture atlas resolution %i:%i\n", currentAtlasWidth, currentAtlasHeight);

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
		for (size_t imgId = 0; imgId < scaledImages.size(); imgId++) {
			const Image &scaledImage = scaledImages[imgId];
			int rectId = scaledImage.rectId;
			if (scaledImage.duplicate >= 0) {
				rectId = scaledImages[scaledImage.duplicate].rectId;
			}
			assert(rectId >= 0);
			const Rect opaqueRect = findOpaqueRect(scaledImage.data, scaledImage.width, scaledImage.height, 4, cfg);
			const int x = rects[rectId].x;
			const int y = rects[rectId].y;
			if (rects[rectId].was_packed) {
				const int n = scaledImage.width * 4;
				// TODO: only copy the opaque part of the image?
				for (int row = 0; row < scaledImage.height; row++) {
					uint8_t *dest = atlas + (intptr_t)((y + row) * currentAtlasWidth + x) * 4;
					const uint8_t *src = scaledImage.data + (intptr_t)(row * n);
					memcpy(dest, src, n);
				}
			}

			const std::string &spriteId = extractBasenameNoExtension(scaledImage.name);

			fprintf(luaFile, "\t[");
			fprintf(luaFile, "\"%s\"", spriteId.c_str());
			fprintf(luaFile, "] = {\n");
			fprintf(luaFile, "\t\timage = \"%s\",\n", baseTextureName.c_str());
			fprintf(luaFile, "\t\tx0 = %f,\n", (float)x / (float)currentAtlasWidth);
			fprintf(luaFile, "\t\ty0 = %f,\n", (float)y / (float)currentAtlasHeight);
			fprintf(luaFile, "\t\tx1 = %f,\n", (float)scaledImage.width / (float)currentAtlasWidth);
			fprintf(luaFile, "\t\ty1 = %f,\n", (float)scaledImage.height / (float)currentAtlasHeight);
			fprintf(luaFile, "\t\ttrimmedoffsetx = %i,\n", opaqueRect.x);
			fprintf(luaFile, "\t\ttrimmedoffsety = %i,\n", opaqueRect.y);
			fprintf(luaFile, "\t\ttrimmedwidth = %i,\n", opaqueRect.width);
			fprintf(luaFile, "\t\ttrimmedheight = %i,\n", opaqueRect.height);
			fprintf(luaFile, "\t\tuntrimmedwidth = %i,\n", scaledImage.width);
			fprintf(luaFile, "\t\tuntrimmedheight = %i,\n", scaledImage.height);
			fprintf(luaFile, "\t},\n");
		}
		fprintf(luaFile, "}\n");
		fclose(luaFile);

		// Save the atlas
		stbi_write_png(v.textureFilename.c_str(), currentAtlasWidth, currentAtlasHeight, 4, atlas,
					   currentAtlasWidth * 4);
		if (cfg.pngQuant) {
			pngQuant(v.textureFilename);
		}
		free(atlas);
		return true;
	}
	error_printf("Failed to pack texture atlas\n");
	return false;
}

// mark the second and third version of the same image (same hash as duplicate)
// but not the first one
static void markDuplicates(std::vector<Image> &images) {
	for (size_t i = 0; i < images.size(); i++) {
		if (images[i].duplicate >= 0) {
			continue;
		}
		for (size_t j = i + 1; j < images.size(); j++) {
			if (images[j].duplicate >= 0) {
				continue;
			}
			if (images[i].hash == images[j].hash) {
				verbose_printf(" * Found duplicate: %s - %i (of %s - %i)\n", images[j].name.c_str(), (int)j,
							   images[i].name.c_str(), (int)i);
				images[j].duplicate = (int)i;
			}
		}
	}
}

static bool loadTps(const std::string &tpsFile, Config &cfg) {
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

	const pugi::xpath_node &luaNode = doc.select_node("//key[text()='lua']/following-sibling::struct/filename");
	const std::string &luaFilename = luaNode.node().text().as_string();
	verbose_printf("* Lua file template: %s\n", luaFilename.c_str());
	verbose_printf("* Texture file template: %s\n", textureFilename.c_str());

	const pugi::xpath_node &spriteSettingsNode = doc.select_node("//key[text()='globalSpriteSettings']/following-sibling::struct");
	if (spriteSettingsNode) {
		const std::string &trimMode = readString(spriteSettingsNode.node(), "trimMode", "None", "enum");
		if (trimMode == "None") {
			cfg.trimMode = TrimMode::None;
		} else if (trimMode == "Trim") {
			cfg.trimMode = TrimMode::Trim;
		} else {
			error_printf("Unknown TrimMode: %s\n", trimMode.c_str());
			return false;
		}
		verbose_printf("* TrimMode: %s\n", trimMode.c_str());

		const std::string &pivotPoint = readString(spriteSettingsNode.node(), "pivotPoint", "Center", "enum");
		verbose_printf("* PivotPoint: %s\n", pivotPoint.c_str());
	} else {
		verbose_printf("No SpriteSettings struct found\n");
	}

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

		int maxTextureWidth = 2048;
		int maxTextureHeight = 2048;
		const std::string xpathWidth =
			"./key[text()='maxTextureSize']/following-sibling::QSize/key[text()='width']/following-sibling::int";
		const std::string xpathHeight =
			"./key[text()='maxTextureSize']/following-sibling::QSize/key[text()='height']/following-sibling::int";
		pugi::xpath_node_set widthNode = structNode.select_nodes(xpathWidth.c_str());
		pugi::xpath_node_set heightNode = structNode.select_nodes(xpathHeight.c_str());
		if (!widthNode.empty() && !heightNode.empty()) {
			maxTextureWidth = widthNode.first().node().text().as_int();
			maxTextureHeight = heightNode.first().node().text().as_int();
		}
		variants.emplace_back(scale, extension, textureFilenameVariant, luaFilenameVariant, maxTextureWidth,
							  maxTextureHeight);
		verbose_printf("* Found variant: scale: %f, extension: %s\n", scale, extension.c_str());
	}
	if (variants.empty()) {
		error_printf("No variants found\n");
		return false;
	}

	const pugi::xpath_node_set &tpsFiles =
		doc.select_nodes("//key[text()='fileList']/following-sibling::array/filename");
	std::vector<std::string> input_files;
	verbose_printf("* Images:\n");
	for (const pugi::xpath_node &file : tpsFiles) {
		const std::string &imageName = file.node().text().as_string();
		verbose_printf("  * Image file: %s\n", imageName.c_str());
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
		images[i].hash = hash(images[i].data, images[i].width * images[i].height * 4, 0);
	}

	std::sort(images.begin(), images.end(), [](const Image &a, const Image &b) {
		return extractBasenameNoExtension(a.name) < extractBasenameNoExtension(b.name);
	});

	markDuplicates(images);

	for (const Variants &v : variants) {
		if (!handleVariant(v, images, cfg)) {
			return false;
		}
	}
	return true;
}

static void usage(const char *appname) {
	fprintf(stderr, "Usage: %s [-a 0-255] [-d] [-h] [-i 32] [-p] [-v] [file.tps]...\n\n", appname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -a <threshold>\tAlpha threshold\n");
	fprintf(stderr, "  -d\tEnable debug output\n");
	fprintf(stderr, "  -h\tShow this help\n");
	fprintf(stderr, "  -i <value>\tIncrease the texture atlas size by the specified amount\n");
	fprintf(stderr, "  -p\tUse pngquant to compress the texture atlas. pngquant tool must be in the path\n");
	fprintf(stderr, "  -v\tEnable verbose output\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This tool reads a TexturePacker .tps file and creates the texture atlas\n"
					" as well as the lua scripts.\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	const char *appname = argv[0];
	size_t optsParsed = 1;
	Config cfg;

	for (optsParsed = 1; optsParsed < argc && argv[optsParsed][0] == '-'; optsParsed++) {
		switch (argv[optsParsed][1]) {
		case 'd':
			g_debug = true;
			break;
		case 'v':
			g_verbose = true;
			break;
		case 'a': {
			if (optsParsed + 1 >= argc) {
				usage(appname);
			}
			cfg.alphaThreshold = atoi(argv[optsParsed + 1]);
			optsParsed++;
			break;
		}
		case 'p':
			cfg.pngQuant = true;
			break;
		case 'i': {
			if (optsParsed + 1 >= argc) {
				usage(appname);
			}
			g_atlasIncrease = atoi(argv[optsParsed + 1]);
			optsParsed++;
			break;
		}
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

	char cwd[1024] = "";
	getcwd(cwd, sizeof(cwd));
	verbose_printf("Current working directory: %s\n", cwd);

	for (int i = 0; i < argc; i++) {
		const std::string tpsFile = argv[i];
		verbose_printf("Processing %s\n", tpsFile.c_str());

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
		debug_printf("Changed directory to %s\n", path.c_str());
		// extract the filename
		std::string filename = tpsFile.substr(pos + 1);
		if (!loadTps(filename, cfg)) {
			error_printf("Failed to handle %s\n", tpsFile.c_str());
			return 1;
		}

		if (i < argc - 1) {
			if (chdir(cwd) != 0) {
				error_printf("Failed to chdir to %s\n", cwd);
				return 1;
			}
			debug_printf("Changed directory to %s\n", cwd);
		}
	}
	return 0;
}
