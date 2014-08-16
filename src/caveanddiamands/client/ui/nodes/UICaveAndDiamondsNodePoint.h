#pragma once

#include "engine/client/ui/nodes/UINodePoint.h"

class UICaveAndDiamondsNodePoint: public UINodePoint {
private:
	int _ownBest;
	int _globalBest;
public:
	UICaveAndDiamondsNodePoint(IFrontend *frontend, uint32_t updateDelay = 10);
	virtual ~UICaveAndDiamondsNodePoint();

	void setOwnAndGlobalBest (int ownBest, int globalBest);
	void setLabel (const std::string& label) override;
};

inline void UICaveAndDiamondsNodePoint::setOwnAndGlobalBest (int ownBest, int globalBest) {
	_ownBest = ownBest;
	_globalBest = globalBest;
}
