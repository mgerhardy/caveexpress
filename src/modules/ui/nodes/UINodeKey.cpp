#include "UINodeKey.h"
#include "common/IFrontend.h"
#include "common/Math.h"

UINodeKey::UINodeKey (IFrontend *frontend, const std::string& keyName, float x, float y, float height) :
		UINodeLabel(frontend, "  "+keyName+"  ",
			getFont(keyName.length() > 1 ? MEDIUM_FONT : HUGE_FONT))  // 1 char key bigger
{
	setBorder(true);
	setBorderColor(colorGray);
	setColor(colorWhite);
	setBackgroundColor(colorBlack);

	setPos(x, y - height * 0.5f);
	setSize(0.0f, height * 2.f);
	autoSize();
}

UINodeKey::~UINodeKey ()
{
}

float UINodeKey::getAutoHeight() const
{
	return getHeight();
}
