/**
 * @file
 * @brief Generate a Tiled mapeditor tileset from the CaveExpress sprites.
 */

#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "common/Application.h"
#include "common/ConfigManager.h"
#include "common/File.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/Math.h"
#include "common/SpriteDefinition.h"
#include "common/String.h"
#include "common/TextureDefinition.h"
#include "common/ThemeType.h"

extern "C" int main(int argc, char *argv[]) {
	Application &app = Singleton<Application>::getInstance();
	app.setOrganisation("caveproductions");
	app.setName("caveexpress");

	const auto &tt = ThemeTypes::ROCK;
	const auto &waterFall = caveexpress::SpriteTypes::WATERFALL;
	Log::debug(LOG_MAIN, "Theme: %s", tt.name.c_str());
	Log::debug(LOG_MAIN, "Sprite: %s", waterFall.name.c_str());

	Config.init(nullptr, argc, argv);

	const std::string themeNameArg = argc >= 2 ? argv[1] : "";
	const ThemeType &themeArg = ThemeType::getByName(themeNameArg);
	std::vector<const ThemeType *> themes;
	if (themeArg.isNone()) {
		for (auto iter = ThemeType::begin(); iter != ThemeType::end(); ++iter) {
			if (iter->second->isNone()) {
				continue;
			}
			themes.push_back(iter->second);
		}
	} else {
		themes.push_back(&themeArg);
	}

	TextureDefinition t("big");
	if (t.getSize() == 0u) {
		Log::error(LOG_MAIN, "No textures found");
		return EXIT_FAILURE;
	}
	SpriteDefinition::get().init(t);

	for (auto themeIter = themes.begin(); themeIter != themes.end(); ++themeIter) {
		const ThemeType &theme = **themeIter;
		const std::string &themeName = theme.name;
		std::string tsx = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		// TODO: width height
		tsx += "<tileset version=\"1.8\" tiledversion=\"1.8.2\" name=\"";
		tsx += "caveexpress-tileset-" + themeName;
		tsx += "\" tilewidth=\"1024\" tileheight=\"1024\" tilecount=\"${tilecount}\" columns=\"0\">\n";
		tsx += " <grid orientation=\"orthogonal\" width=\"1\" height=\"1\"/>\n";

		// only add one of the images to the tileset - the first one found - let's start with the middle layer
		const Layer layers[] = {LAYER_MIDDLE, LAYER_BACK, LAYER_FRONT, LAYER_FRONT_1, LAYER_FRONT_2};
		static_assert(lengthof(layers) == MAX_LAYERS, "Invalid layer count");

		int tileId = 0;
		for (auto &spriteDefIter : SpriteDefinition::get()) {
			const std::shared_ptr<SpriteDef> &def = spriteDefIter.second;
			if (def->theme != theme) {
				continue;
			}

			for (int n = 0; n < lengthof(layers); ++n) {
				const SpriteDef::SpriteDefFrames &textures = def->textures[layers[n]];
				if (textures.empty()) {
					Log::debug(LOG_MAIN, "No textures for %s on layer %i", def->id.c_str(), layers[n]);
					continue;
				}
				const SpriteDefFrame &spriteDefFrame = textures.front();
				const std::string &texture = spriteDefFrame.name;
				tsx += string::format(" <tile id = \"%i\">\n", tileId);
				tsx += string::format(
					"  <image width = \"%i\" height = \"%i\" source = \"../png/caveexpress/%s.png\" />\n",
					(int)def->width, (int)def->height, texture.c_str());
				vec2 size(def->width, def->height);
				if (def->hasShape()) {
					size = def->calculateSizeFromShapeData();
					auto mins = def->getMins();
					auto maxs = def->getMaxs();
					tsx += "  <objectgroup draworder=\"index\" id=\"2\">\n";
					tsx += string::format("   <object id=\"1\" x=\"%i\" y=\"%i\" width=\"%i\" height=\"%i\">\n",
										  (int)mins.x, (int)mins.y, (int)(mins.x + maxs.x), (int)(mins.y + maxs.y));
					if (def->polygons.size() > 0) {
						tsx += "    <polygon points=\"";
						for (const SpritePolygon &polygon : def->polygons) {
							for (const SpriteVertex &vertex : polygon.vertices) {
								const float x = vertex.x;
								const float y = vertex.y;
								tsx += string::format("%i,%i ", (int)x, (int)y);
							}
						}
						tsx += "\"/>\n";
						tsx += "   </object>\n";

						tsx += "  </objectgroup>\n";
					}
				}
#if 0
				tsx += string::format("  <properties>\n");
				tsx += string::format("   <property name=\"foo\" value=\"bar\"/>\n");
				tsx += string::format("  </properties>\n");
#endif
				tsx += " </tile>\n";
				++tileId;
				break;
			}
		}

		tsx += "</tileset>\n";

		tsx = string::replaceAll(tsx, "${tilecount}", string::toString(tileId));

		const std::string &filename =
			string::format("contrib/assets/tiled/caveexpress-tileset-%s.tsx", themeName.c_str());
		FS.writeSysFile(filename, (const unsigned char *)tsx.c_str(), tsx.size(), true);
	}

	return EXIT_SUCCESS;
}
