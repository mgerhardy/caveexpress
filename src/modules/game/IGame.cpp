#include "IGame.h"
#include "common/Achievement.h"
#include "network/INetwork.h"

int IGame::getMaxClients() {
	return MAX_CLIENTS;
}

void IGame::onCampaignUnlock(Campaign* oldCampaign, Campaign* newCampaign) {
	if (oldCampaign == nullptr) {
		Log::debug(LOG_GAME, "no old campaign available while unlocking %s", newCampaign->getId().c_str());
		return;
	}

	const std::string& id = oldCampaign->getSetting("achievement");
	if (id.empty()) {
		Log::debug(LOG_GAME, "no achievement for unlocking %s", oldCampaign->getId().c_str());
		return;
	}

	const Achievement& achievement = Achievement::getByName(id);
	if (!achievement) {
		Log::error(LOG_GAME, "invalid achievement for unlocking %s => %s",  oldCampaign->getId().c_str(), id.c_str());
		return;
	}

	achievement.unlock();
}
