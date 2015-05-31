#pragma once

#include "client/ui/nodes/UINodePoint.h"

class UICavePackerNodePoint: public UINodePoint {
private:
	int _ownBest;
	int _globalBest;
public:
	UICavePackerNodePoint(IFrontend *frontend, uint32_t updateDelay = 10);
	virtual ~UICavePackerNodePoint();

	void setOwnAndGlobalBest (int ownBest, int globalBest);
	void setLabel (const std::string& label) override;
};

inline void UICavePackerNodePoint::setOwnAndGlobalBest (int ownBest, int globalBest) {
	_ownBest = ownBest;
	_globalBest = globalBest;
}
