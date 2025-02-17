package org.cavepacker;

import java.util.List;

import org.base.game.BaseGameActivity;

/**
 * Activity for the free version of the game with payment support from its super
 * class and ads
 */
public class CavePacker extends BaseGameActivity {
	@Override
	public String getName() {
		return "cavepacker";
	}

	@Override
	protected String getTrackerId() {
		return "UA-54860800-2";
	}
}
