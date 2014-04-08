#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <list>
#include <string>
#include <string.h>
#include <iostream>
#include <Box2D/Common/b2Math.h>
#include <SDL.h>
#include <yajl/yajl_parse.h>
#include "engine/common/Common.h"
#include "engine/common/File.h"

const b2Vec2 Vec2Zero(0.0f, 0.0f);
#define VERTICESINDENT "\t\t\t\t"

static int polygonDepth = -1;
static int originDepth = -1;
static int arrayDepth = 0;
static int depth;
static int array;
static int state;
static int vertices = 0;
static float scale = 100.0f;
static float originX = 0.0f;
static float originY = 0.0f;
static int coordinatesParsed = 0;
static std::string onlyName;

static const int PARSEVERTICES = 1 << 1;
static const int PARSENAME = 1 << 2;
static const int WRITEPOLYGON = 1 << 3;
static const int PARSEORIGIN = 1 << 4;
static const int PARSEORIGINX = 1 << 5;
static const int PARSEORIGINY = 1 << 6;
static const int SKIP = 1 << 7;

static void printError (yajl_handle hand, const std::string& str)
{
	unsigned char * error = yajl_get_error(hand, 1, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	std::cerr << reinterpret_cast<const char *>(error) << std::endl;
	yajl_free_error(hand, error);
}

static int convert_null (void * ctx)
{
	return 1;
}

static int convert_boolean (void * ctx, int boolean)
{
	return 1;
}

static int convert_string (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	const std::string str(reinterpret_cast<const char *>(stringVal), stringLen);
	if (state & PARSENAME) {
		if (!onlyName.empty() && str != onlyName) {
			state |= SKIP;
			return 1;
		} else {
			state &= ~SKIP;
		}
		if (!(state & SKIP)) {
			std::cout << "\t[\"" << str << "\"] = {" << std::endl;
		}
		state &= ~PARSENAME;
	}
	return 1;
}

static int convert_number (void * ctx, const char * numberVal, size_t numberLen)
{
	const std::string str(reinterpret_cast<const char *>(numberVal), numberLen);
	if (state & PARSEVERTICES) {
		if (vertices >= 8) {
			vertices = 0;
			if (!(state & SKIP)) {
				std::cout << std::endl << VERTICESINDENT;
			}
		}
		const float origin = ++coordinatesParsed % 2 ? originX : originY;
		const float f = (atof(str.c_str()) - origin) * scale;
		vertices++;
		if (vertices != 1) {
			if (!(state & SKIP)) {
				std::cout << " ";
			}
		}
		if (!(state & SKIP)) {
			std::cout << f << ",";
		}
	} else if (state & PARSEORIGINX) {
		originX = atof(str.c_str());
	} else if (state & PARSEORIGINY) {
		originY = atof(str.c_str());
	}
	return 1;
}

static int convert_map_key (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	const std::string str(reinterpret_cast<const char *>(stringVal), stringLen);
	if (str == "polygons") {
		if (!(state & SKIP)) {
			std::cout << "\t\tpolygons = {" << std::endl;
		}
		state |= PARSEVERTICES;
		vertices = 0;
		polygonDepth = depth;
		arrayDepth = array + 1;
	} else if (str == "name") {
		state |= PARSENAME;
	} else if (str == "origin") {
		state |= PARSEORIGIN;
		originDepth = depth;
	}

	if (state & PARSEORIGIN) {
		state &= ~(PARSEORIGINX | PARSEORIGINY);
		if (str == "x") {
			state |= PARSEORIGINX;
		} else if (str == "y") {
			state |= PARSEORIGINY;
		}
	}

	return 1;
}

static int convert_start_map (void * ctx)
{
	depth++;
	return 1;
}

static int convert_end_map (void * ctx)
{
	depth--;
	if (depth < polygonDepth && (state & PARSEVERTICES)) {
		polygonDepth = -1;
		vertices = 0;
		state &= ~PARSEVERTICES;
		if (!(state & SKIP)) {
			std::cout << std::endl << "\t\t}," << std::endl;
		}
	} else if (depth == 1) {
		if (!(state & SKIP)) {
			std::cout << "\t}," << std::endl;
		}
	}

	if (state & PARSEORIGIN) {
		state &= ~(PARSEORIGINX | PARSEORIGINY | PARSEORIGIN);
	}
	return 1;
}

static int convert_start_array (void * ctx)
{
	array++;
	if (state & WRITEPOLYGON) {
		if (!(state & SKIP)) {
			std::cout << "\t\t\t{" << std::endl << VERTICESINDENT << "\"\", ";
		}
	} else if (state & PARSEVERTICES) {
		state |= WRITEPOLYGON;
	}
	return 1;
}

static int convert_end_array (void * ctx)
{
	array--;
	if (state & PARSEVERTICES) {
		vertices = 0;
		if (array < arrayDepth) {
			state &= ~PARSEVERTICES;
			state &= ~WRITEPOLYGON;
			if (!(state & SKIP)) {
				std::cout << "\t\t}," << std::endl;
			}
		} else {
			if (!(state & SKIP)) {
				std::cout << std::endl << "\t\t\t}," << std::endl;
			}
		}
	}
	return 1;
}

static yajl_callbacks callbacks = { convert_null, convert_boolean, nullptr, nullptr, convert_number, convert_string,
		convert_start_map, convert_map_key, convert_end_map, convert_start_array, convert_end_array };

bool readJson (const std::string& str)
{
	yajl_handle hand = yajl_alloc(&callbacks, nullptr, nullptr);
	yajl_status stat = yajl_parse(hand, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	stat = yajl_complete_parse(hand);
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	yajl_free(hand);

	return true;
}

static void usage ()
{
	std::cerr << "jsonconverter" << std::endl;
	std::cerr << "  --scale <scale>        - set the vertex scale factor" << std::endl;
	std::cerr << "  --name <name>          - skip all sprites, but the one given in <name>" << std::endl;
	std::cerr << "  --help                 - show this help message" << std::endl;
}

extern "C" int main (int argc, char* argv[])
{
	const URI uri("file://./contrib/box2deditor/caveexpress.json");
	File file(uri, SDL_RWFromFile(uri.getPath().c_str(), "rb"), "./contrib/box2deditor/caveexpress.json");
	char *buffer;
	const int fileLen = file.read((void **) &buffer);
	const ScopedArrayPtr<char> p(buffer);
	std::cout.precision(3);
	if (!buffer || fileLen <= 0) {
		std::cerr << "Could not read the json file" << std::endl;
		return EXIT_FAILURE;
	}

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--scale")) && argc >= i + 1) {
				scale = atof(argv[++i]);
				//std::cerr << "use a scale factor of " << scale << std::endl;
			} else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
				usage();
				return EXIT_FAILURE;
			} else if ((!strcmp(argv[i], "--name") || !strcmp(argv[i], "-n")) && argc >= i + 1) {
				onlyName = argv[++i];
			} else {
				std::cerr << "unknown command given: " << argv[i] << std::endl;
				usage();
				return EXIT_FAILURE;
			}
		}
	}

	const std::string str(buffer, fileLen);
	std::cout << "sprites = {" << std::endl;
	const bool ret = readJson(str);
	std::cout << "}" << std::endl;
	if (!ret)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
