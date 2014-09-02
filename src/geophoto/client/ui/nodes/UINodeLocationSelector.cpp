#include "UINodeLocationSelector.h"
#include "engine/common/Logger.h"
#include "engine/common/String.h"

UINodeLocationSelector::UINodeLocationSelector(IFrontend *frontend, const GameRound& gameRound) :
		UINodeSelector<Location>(frontend, 1, 2, 128, 128), _gameRound(gameRound)
{
	reset();
	setColSpacing(0);
	setRowSpacing(2);
	setPadding(0.0f);
	setFont("");
}

UINodeLocationSelector::~UINodeLocationSelector() {
}

bool UINodeLocationSelector::onSelect (const Location& data)
{
	return true;
}

void UINodeLocationSelector::renderSelectorEntry (int index, const Location& location, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	_font->print(string::toString(location.score), colorBlack, x, y);
	_font->print(location.location, colorBlack, x, y + 10);

	const URI thumbUrl(location.thumbUrl);
	FilePtr imagePtr = FS.get().getFile(thumbUrl);
	if (!imagePtr) {
		error(LOG_CLIENT, "could not load the thumbnail for " + location.thumbUrl);
		return;
	}
	TexturePtr t = loadTexture(imagePtr->getFileName());
	if (!t) {
		error(LOG_CLIENT, "could not load the thumbnail texture for " + location.thumbUrl);
		return;
	}

	renderImage(t, x, y, colWidth, rowHeight, alpha);
}

void UINodeLocationSelector::reset ()
{
	UINodeSelector<Location>::reset();
	const Locations& locations = _gameRound.getLocations();
	for (Locations::const_iterator i = locations.begin(); i != locations.end(); ++i) {
		addData(*i);
	}
}
