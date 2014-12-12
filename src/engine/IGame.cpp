#include "IGame.h"
#include "engine/common/Achievement.h"

void IGame::onCampaignUnlock(Campaign* oldCampaign, Campaign* newCampaign)
{
	if (oldCampaign == nullptr) {
		debug(LOG_SERVER, "no old campaign available while unlocking " + newCampaign->getId());
		return;
	}

	const std::string& id = oldCampaign->getSetting("achievement");
	if (id.empty()) {
		debug(LOG_SERVER, "no achievement for unlocking " + oldCampaign->getId());
		return;
	}

	const Achievement& achievement = Achievement::getByName(id);
	if (!achievement) {
		error(LOG_SERVER, "invalid achievement for unlocking " + oldCampaign->getId() + " => " + id);
		return;
	}

	achievement.unlock();
}
