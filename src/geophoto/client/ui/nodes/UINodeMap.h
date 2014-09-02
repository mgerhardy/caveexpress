#pragma once

#include "engine/client/ui/nodes/UINode.h"
#include "engine/client/ui/BitmapFont.h"
#include "geophoto/client/round/GameRound.h"

class RoundController;

class UINodeMap: public UINode {
private:
	RoundController& _controller;

	BitmapFontPtr _font;

	TexturePtr _resultPanelLeft;
	TexturePtr _resultPanelRight;

	// calculates the distance between the given longitudes and latitudes
	double getDistance (float long1, float lat1, float long2, float lat2) const;

	void renderFailed (int x, int y) const;
	void renderMarkers (int x, int y) const;
	void renderResult (int x, int y) const;

public:
	UINodeMap (IFrontend *frontend, RoundController& controller);
	virtual ~UINodeMap ();

	// update the states and timers
	void update (uint32_t deltaTime) override;
	// set the initial state
	bool onPush () override;
	// handles mouse clicks, as well as finger pressed or joystick button presses
	void handleDrop (uint16_t x, uint16_t y);
	void render (int x, int y) const override;
	const TexturePtr& setImage (const std::string& texture) override;
	// focus is not wanted in each of the states
	bool wantFocus () const;

	void setEarth ();
	void setLoading ();
	bool fadeIn (uint32_t deltaTime, uint32_t fadeInMillis);
};

inline void UINodeMap::setEarth ()
{
	setImage("earth");
	setSize(1.0f, getSpacingIntervalY(16));
	setAlignment(NODE_ALIGN_MIDDLE|NODE_ALIGN_CENTER);
}

inline void UINodeMap::setLoading ()
{
	setImage("loading");
}

inline bool UINodeMap::fadeIn (uint32_t deltaTime, uint32_t fadeInMillis)
{
	const float alpha = deltaTime / static_cast<float>(fadeInMillis);
	const float newAlpha = _alpha + alpha;
	setAlpha(newAlpha);
	return fequals(_alpha, 1.0f);
}
