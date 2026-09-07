#include "caveexpress/shared/CaveExpressConfig.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "common/ConfigManager.h"
#include <SDL_stdinc.h>

namespace caveexpress {

void registerCaveExpressConfigVars ()
{
	struct {
		const char *configVar;
		const char *value;
		int flags;
	} gameConfigVars[] = {
		{MAX_HITPOINTS, "100", CV_NOPERSIST},
		{DAMAGE_THRESHOLD, "0.3", CV_NOPERSIST},
		{REFERENCE_TIME_FACTOR, "1.0", CV_NOPERSIST},
		{FRUIT_COLLECT_DELAY_FOR_A_NEW_LIFE, "15000", CV_NOPERSIST},
		{AMOUNT_OF_FRUITS_FOR_A_NEW_LIFE, "4", CV_NOPERSIST},
		{FRUIT_HITPOINTS, "10", CV_NOPERSIST},
		{WORLD_PARTICLE, "true", CV_READONLY | CV_NOPERSIST},
		{NPC_FLYING_SPEED, "2.0", CV_NOPERSIST},
		{FLYING_SPEED_X, "1.0", CV_NOPERSIST}
	};

	const int n = SDL_arraysize(gameConfigVars);
	for (int i = 0; i < n; ++i) {
		Config.initOrGetConfigVar(gameConfigVars[i].configVar, gameConfigVars[i].value, gameConfigVars[i].flags);
	}

	// we have to override this - otherwise the old value from the config is used... which would be bad
	Config.getConfigVar(NPC_FLYING_SPEED)->setValue("2.0");
}

}
