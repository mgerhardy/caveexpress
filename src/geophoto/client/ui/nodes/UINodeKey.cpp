#include "UINodeKey.h"

UINodeKey::UINodeKey (IFrontend *frontend, const std::string& keyName, float x, float y, float height) :
		UINodeLabel(frontend, keyName)
{
	setBorder(true);
	setBorderColor(colorBlack);
	setBackgroundColor(colorWhite);

	setPos(x, y);
	setSize(0.0f, height);
	autoSize();
}

UINodeKey::~UINodeKey ()
{
}

float UINodeKey::getAutoHeight() const
{
	return getHeight();
}
