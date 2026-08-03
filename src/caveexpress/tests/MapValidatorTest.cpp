#include "tests/TestShared.h"
#include "caveexpress/shared/MapValidator.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/server/map/WfcMapGenerator.h"
#include "common/ThemeType.h"
#include "common/TextureDefinition.h"
#include "common/SpriteDefinition.h"
#include "common/MapSettings.h"
#include "common/Log.h"
#include "common/String.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace caveexpress {

class MapValidatorTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}

	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}

	MapMetrics evaluateContext (CaveExpressMapContext& ctx) const
	{
		const int w = string::toInt(ctx.getSettings().at(msn::WIDTH));
		const int h = string::toInt(ctx.getSettings().at(msn::HEIGHT));
		return MapValidator().evaluate(w, h, ctx.getMapTileDefinitions(), ctx.getCaveTileDefinitions(),
				ctx.getEmitterDefinitions(), ctx.getStartPositions());
	}

	MapMetrics evaluateWfc (const WfcMapGenerator::Result& result) const
	{
		const int w = string::toInt(result.settings.at(msn::WIDTH));
		const int h = string::toInt(result.settings.at(msn::HEIGHT));
		return MapValidator().evaluate(w, h, result.tiles, result.caves, result.emitters, result.startPositions);
	}
};

TEST_F(MapValidatorTest, testHandMapBaseline)
{
	const char* maps[] = {
		"rock-01", "rock-08", "wind-01", "wind-02", "wind-03",
		"villages-01", "villages-08", "third-ice-01", "third-ice-05"
	};

	std::vector<float> scores;
	std::vector<float> exposed;
	std::vector<float> orphans;
	int hardPass = 0;

	for (const char* name : maps) {
		CaveExpressMapContext ctx(name);
		ASSERT_TRUE(ctx.load(true)) << name;
		const MapMetrics m = evaluateContext(ctx);
		Log::info(LOG_GAMEIMPL,
				"hand map %s score=%.1f valid=%i exposed=%.3f orphan=%.3f caves=%i/%i pkgTarget=%i walkSurf=%.3f",
				name, m.totalScore, m.valid ? 1 : 0, m.exposedRockTopRatio, m.orphanColliderRatio,
				m.cavesReachable, m.caveCount, m.packageTargetCount, m.walkableSurfaceRatio);
		scores.push_back(m.totalScore);
		exposed.push_back(m.exposedRockTopRatio);
		orphans.push_back(m.orphanColliderRatio);
		if (m.valid)
			++hardPass;
		// Hand maps should mostly pass hard reachability
		EXPECT_TRUE(m.valid || m.caveCount == 0) << name << ": " << m.failureReason;
	}

	ASSERT_FALSE(scores.empty());
	std::sort(scores.begin(), scores.end());
	std::sort(exposed.begin(), exposed.end());
	std::sort(orphans.begin(), orphans.end());
	const float scoreP10 = scores[std::max<size_t>(0, scores.size() / 10)];
	const float exposedP90 = exposed[std::min(exposed.size() - 1, (exposed.size() * 9) / 10)];
	const float orphanP90 = orphans[std::min(orphans.size() - 1, (orphans.size() * 9) / 10)];
	Log::info(LOG_GAMEIMPL, "hand baseline scoreP10=%.1f exposedP90=%.3f orphanP90=%.3f hardPass=%i/%i",
			scoreP10, exposedP90, orphanP90, hardPass, (int)scores.size());

	EXPECT_GE(hardPass, static_cast<int>(scores.size()) - 2);
	EXPECT_GE(scoreP10, 30.0f);
}

TEST_F(MapValidatorTest, testWfcMetricsVsHandBaseline)
{
	// Thresholds seeded from typical hand-map quality (relaxed slightly for procedural).
	const float minScore = 40.0f;
	const float maxExposedRock = 0.55f;
	const float maxOrphan = 0.25f;

	int success = 0;
	int metricPass = 0;
	float scoreSum = 0.0f;
	const int attempts = 24;

	WfcRules rules = WfcRules::loadFromLua();
	for (int i = 0; i < attempts; ++i) {
		const ThemeType& theme = (i & 1) ? ThemeTypes::ICE : ThemeTypes::ROCK;
		WfcMapGenerator gen(theme, 18, 12, static_cast<unsigned>(std::max(2, rules.caveTarget)), rules);
		const WfcMapGenerator::Result result = gen.generate(static_cast<unsigned int>(2000 + i * 17));
		if (!result.success)
			continue;
		++success;
		const MapMetrics m = evaluateWfc(result);
		scoreSum += m.totalScore;
		Log::info(LOG_GAMEIMPL,
				"wfc seed=%i score=%.1f valid=%i exposed=%.3f orphan=%.3f caves=%i pkgBadNiche=%i winAdj=%i (%s)",
				2000 + i * 17, m.totalScore, m.valid ? 1 : 0, m.exposedRockTopRatio, m.orphanColliderRatio,
				m.caveCount, m.packageTargetsWithBadNiche, m.windowWindowAdjacencies, m.failureReason.c_str());

		const bool ok = m.valid
				&& m.totalScore >= minScore
				&& m.exposedRockTopRatio <= maxExposedRock
				&& m.orphanColliderRatio <= maxOrphan
				&& m.windowWindowAdjacencies == 0
				&& m.cavesTooClose == 0;
		if (ok)
			++metricPass;
	}

	EXPECT_GE(success, attempts * 2 / 3) << "WFC success rate too low";
	EXPECT_GE(metricPass, success / 2) << "WFC metric pass rate too low vs hand baseline";
	if (success > 0) {
		const float avg = scoreSum / static_cast<float>(success);
		Log::info(LOG_GAMEIMPL, "wfc avgScore=%.1f metricPass=%i/%i", avg, metricPass, success);
		EXPECT_GE(avg, minScore - 5.0f);
	}
}

}
