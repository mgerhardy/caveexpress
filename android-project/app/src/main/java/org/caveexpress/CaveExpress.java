package org.caveexpress;

import org.base.game.BaseGameActivity;

/**
 * Activity for the free version of the game with payment support from its super
 * class and ads
 */
public class CaveExpress extends BaseGameActivity {
	@Override
	public String getName() {
		return "caveexpress";
	}

	@Override
	protected String getTrackerId() {
		return "UA-54860800-3";
	}
}
