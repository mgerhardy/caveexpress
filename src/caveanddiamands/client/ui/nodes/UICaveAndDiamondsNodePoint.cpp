#include "UICaveAndDiamondsNodePoint.h"

UICaveAndDiamondsNodePoint::UICaveAndDiamondsNodePoint(IFrontend *frontend,
		uint32_t updateDelay) : UINodePoint(frontend, updateDelay), _ownBest(0), _globalBest(0) {
}

UICaveAndDiamondsNodePoint::~UICaveAndDiamondsNodePoint() {
}

void UICaveAndDiamondsNodePoint::setLabel (const std::string& label) {
	std::string changedLabel = label;
	if (_ownBest > 0)
		changedLabel += ", " + string::toString(_ownBest);
	else
		changedLabel += ", 0";
	if (_globalBest > 0)
		changedLabel += ", " + string::toString(_globalBest);
	else
		changedLabel += ", 0";

	UINodePoint::setLabel(changedLabel);
}
